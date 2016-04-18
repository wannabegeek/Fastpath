/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#ifndef FASTPATH_BASEMESSAGE_H
#define FASTPATH_BASEMESSAGE_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <limits>
#include <memory>
#include <chrono>

#include "fastpath/messages/Serializable.h"
#include "fastpath/messages/Field.h"
#include "fastpath/Exception.h"
#include "fastpath/utils/stringhash.h"
#include "fastpath/utils/tfpool.h"
#include "fastpath/utils/fast_linear_allocator.h"
#include "fastpath/types.h"
#include "fastpath/messages/ScalarField.h"
#include "fastpath/messages/DataField.h"
#include "fastpath/messages/DateTimeField.h"
#include "fastpath/messages/SmallDataField.h"
#include "fastpath/messages/LargeDataField.h"

namespace fp {
    class MessageField;

    /// @cond DEV
    typedef enum {
        scalar_t,
        data_t,
        message_t
    } DataStorageType;

    /// @endcond

    /**
     * Base class containing the body implementation of a message.
     *
     * You shouldn't use this class directly you should use it derived class Message
     */
    class BaseMessage : public Serializable, public tf::reusable {
    private:
        using payload_type = Field *;
        typedef std::vector<payload_type> PayloadContainer;
        typedef std::unordered_map<const char *, const size_t, tf::string_hash, tf::string_comparator> KeyMappingsContainer;

        static constexpr const uint8_t body_flag = 2;

        PayloadContainer m_payload;
        KeyMappingsContainer m_keys;

        static const DataStorageType getStorageType(const storage_type type);

        using field_allocator_type = tf::linear_allocator<unsigned char>; //std::allocator<unsigned char>;
        field_allocator_type::arena_type m_arena;
        field_allocator_type m_field_allocator;

    protected:
        /// @cond DEV
        /**
         * Helper method for operator<<
         */
        virtual std::ostream& output(std::ostream& out) const;
        /// @endcond

        template <class ...Args> inline ScalarField *createScalarField(Args &&...args) {
            return createField<ScalarField>(std::forward<Args>(args)...);
        }

        template <class ...Args> inline DataField *createDataField(std::size_t size, Args &&...args) {
            if (tf::likely(size <= SmallDataField::max_size)) {
                return createField<SmallDataField>(std::forward<Args>(args)...);
            } else {
                return createField<LargeDataField<field_allocator_type>>(std::forward<Args>(args)..., m_field_allocator);
            }
        }

        template <class ...Args> inline DateTimeField *createDateTimeField(Args &&...args) {
            return createField<DateTimeField>(std::forward<Args>(args)...);
        }

        template <class ...Args> inline MessageField *createMessageField(Args &&...args) {
            return createField<MessageField>(std::forward<Args>(args)...);
        }

        template <typename T, class ...Args> inline T *createField(Args &&...args) {
            std::allocator_traits<field_allocator_type>::pointer ptr = std::allocator_traits<field_allocator_type>::allocate(m_field_allocator, sizeof(T) + sizeof(std::size_t));
            std::size_t *info = reinterpret_cast<std::size_t *>(ptr);

            *info = sizeof(T);
            std::advance(ptr, sizeof(std::size_t));
            return new(ptr) T(std::forward<Args>(args)...);
        }

        template <typename T, class ...Args> inline void destroyField(T *field) noexcept {
            field->~T();

            std::allocator_traits<field_allocator_type>::pointer ptr = reinterpret_cast<std::allocator_traits<field_allocator_type>::pointer>(field);
            std::advance(ptr, -sizeof(std::size_t));
            std::size_t *info = reinterpret_cast<std::size_t *>(ptr);
            *info += sizeof(std::size_t);
            std::allocator_traits<field_allocator_type>::deallocate(m_field_allocator, ptr, *info);
        }

    public:
        /**
         * Constructor
         */
        BaseMessage() : m_arena(8192), m_field_allocator(m_arena) {
            m_payload.reserve(64);
            m_keys.reserve(64);
        }

        /**
         * Move constructor
         */
        BaseMessage(BaseMessage &&msg) noexcept;
        virtual ~BaseMessage();

        BaseMessage(const BaseMessage &msg) = delete;
        BaseMessage& operator=(BaseMessage const&) = delete;
        const bool operator==(const BaseMessage &other) const;
        /**
         * Return the number of fields contained in the message.
         *
         * @return number of fields.
         */
        const uint32_t size() const noexcept { return static_cast<uint32_t>(m_payload.size()); }

        /**
         * Return the storage type for a field
         *
         * @param field The field identifier name.
         * @return The storage type.
         */
        const storage_type storageType(const char *field) const {
            const Field *element = m_payload[m_keys.at(field)];
            return element->type();
        }

        /**
         * Clears all the fields of the message, so it can be re-used.
         */
        virtual void clear();

        /**
         * Adds a scalar field of a type `<T>` to the message.
         *
         * @param field The field identifier name.
         * @param value Sets the field value as this scalar value.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_arithmetic<T>::value>> bool addScalarField(const char *field, const T &value) {
//            void *ptr = m_a.allocate(sizeof(ScalarField));
            auto e = this->createScalarField(field, value);
            auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
            if (result.second) {
                m_payload.emplace_back(e);
            } else {
                this->destroyField(e);
            }
            return result.second;
        }

        /**
         * Adds a data field of a string type to the message.
         *
         * @param field The field identifier name.
         * @param value Sets the field value as this string.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addDataField(const char *field, const char *value);

        /**
         * Adds a data field containing opaque bytes type to the message.
         *
         * @param field The field identifier name.
         * @param value Sets the field value as byte array.
         * @param size The length of the data array to be added.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addDataField(const char *field, const byte *value, const size_t size);

        /**
         * Adds a field containing a sub-message to the message.
         * It is the responsibility of the application to make sure the
         * sub-message is valid until the message has been send successfully.
         *
         * @param field The field identifier name.
         * @param msg Sets the field value to this message.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addMessageField(const char *field, BaseMessage &&msg);

        /**
         * Adds a date-time field to the message.
         *
         * @param field The field identifier name.
         * @param time Sets the field value to this time.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addDateTimeField(const char *field, const std::chrono::time_point<std::chrono::system_clock> &time);

        /**
         * Adds a date-time field to the message.
         *
         * @param field The field identifier name.
         * @param time Sets the field value to this time.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addDateTimeField(const char *field, const std::chrono::microseconds &time);

        /**
         * Adds a date-time field to the message.
         *
         * @param field The field identifier name.
         * @param seconds Sets the field value with seconds.
         * @param microseconds Sets the field value with microseconds.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addDateTimeField(const char *field, const uint64_t seconds, const uint64_t microseconds);

        ////////////// REMOVE ///////////////

        /**
         * Remove a field from the message.
         *
         * @param field The field identifier name.
         * @return `true` if the field was successfully removed, `false` otherwise
         */
        bool removeField(const char *field);


        ////////////// ACCESSOR ///////////////

        template <typename T> bool getScalarField(const char *field, T &value) const {
            if (field != nullptr) {
                auto index = m_keys.find(field);
                if (index != m_keys.end()) {
                    const ScalarField *element = reinterpret_cast<ScalarField *>(
                            m_payload[index->second]);
                    value = element->get<T>();
                    return true;
                }
            }
            return false;
        }

        bool getDataField(const char *field, const char **value, size_t &length) const;

        /**
         * Detach the message from the library.
         * Passes the ownership of the message to the caller, it is now the responsibility
         * of the caller to free up the resources.
         */
        void detach() noexcept;

        /// @cond DEV

        // from Serializable
        virtual const size_t encode(MessageBuffer &buffer) const noexcept override;
        virtual const bool decode(const MessageBuffer::ByteStorageType &buffer) throw (fp::exception) override;

        // from reusable
        void prepareForReuse() override;

        /// @endcond

        /**
         * Output operator for printing.
         * Displays the message in a printable form.
         */
        friend std::ostream &operator<<(std::ostream &out, const BaseMessage &msg) {
            return msg.output(out);
        }
    };
}

#endif //FASTPATH_MESSAGE_H
