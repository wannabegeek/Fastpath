//
// Created by Tom Fewster on 12/02/2016.
//

#ifndef TFDCF_BASEMESSAGE_H
#define TFDCF_BASEMESSAGE_H

#include <cstdint>
#include <string>
#include <vector>
#include <unordered_map>
#include <array>
#include <limits>
#include <memory>
#include <chrono>
#include "Serializable.h"
#include "Field.h"
#include "Exception.h"
#include "utils/tfpool.h"
#include "utils/short_alloc.h"
#include "utils/fast_linear_allocator.h"
#include "types.h"
#include "ScalarField.h"
#include "DataField.h"
#include "DateTimeField.h"
#include "MessageField.h"

/*
 *
 * Header
 * | Msg Length | Flags | Reserverd | Subject Length | Subject |
 * |   8 bytes  |  1 b  |  16 bytes |   4 bytes      |  Var    |
 *
 * Field Map Repeating Block
 * | Num Fields |
 * |  4 bytes   |
 *
 *      | Identifier | Offset  | Name Length | Name |
 *      |  4 bytes   | 8 bytes |  4 bytes    |  Var |
 *
 * Data Segment Repeating Block
 *      | Data Type | Field Length | Data |
 *      |   1 byte  |  8 bytes     |  Var |
 *
 */


namespace DCF {
    /// @cond DEV
    typedef enum {
        scalar_t,
        data_t,
        message_t
    } DataStorageType;

    struct FieldIdentifierComparitor {
        bool operator()(const char *s1, const char *s2) const {
            return strcmp(s1, s2) == 0;
        }
    };

    struct FieldIdentifierHash {
        size_t operator()(const char *s) const {
            size_t result = 0;
            const size_t prime = 31;

            size_t i = 0;
            while (s[i] != '\0') {
                result = s[i] + (result * prime);
                i++;
            }

            return result;
        }
    };

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
        typedef std::unordered_map<const char *, const size_t, FieldIdentifierHash, FieldIdentifierComparitor> KeyMappingsContainer;
//        typedef std::unordered_map<std::string, const size_t> KeyMappingsContainer;

        static constexpr const uint8_t body_flag = 2;

        PayloadContainer m_payload;
        KeyMappingsContainer m_keys;

        static const DataStorageType getStorageType(const StorageType type);

//        tf::linear_allocator<byte> m_a;

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

        using DataFieldType = DataField<field_allocator_type>;
        using DateTimeFieldType = DateTimeField<field_allocator_type>;

        inline payload_type createField(StorageType type) noexcept {
            switch (type) {
                case StorageType::string:
                case StorageType::data:
                    return this->createDataField();
                    break;
                case StorageType::date_time:
                    return this->createDateTimeField();
                    break;
                case StorageType::message:
                    return this->createMessageField();
                    break;
                default:
                    return this->createScalarField();
                    break;
            }
        }

        inline ScalarField *createScalarField() noexcept {
            return createField<ScalarField>();
        }

        inline DataFieldType *createDataField() noexcept {
            return createField<DataFieldType>(m_field_allocator);
        }

        inline DateTimeFieldType *createDateTimeField() noexcept {
            return createField<DateTimeFieldType>(m_field_allocator);
        }

        inline MessageField *createMessageField() noexcept {
            return createField<MessageField>();
        }

        template <typename T, class ...Args> inline T *createField(Args &&...args) noexcept {
            void *ptr = std::allocator_traits<field_allocator_type>::allocate(m_field_allocator, sizeof(T));
            return new(ptr) T(std::forward<Args>(args)...);
        }

        template <typename T, class ...Args> inline void destroyField(T *field) noexcept {
            field->~T();
            std::allocator_traits<field_allocator_type>::deallocate(m_field_allocator, reinterpret_cast<std::allocator_traits<field_allocator_type>::pointer>(field), sizeof(decltype(*field)));
        }

    public:
        /**
         * Constructor
         */
        BaseMessage() : m_arena(4096 * 4), m_field_allocator(m_arena) {
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
        const uint32_t size() const noexcept { return m_payload.size(); }

        /**
         * Return the storage type for a field
         *
         * @param field The field identifier name.
         * @return The storage type.
         */
        const StorageType storageType(const char *field) const {
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
            auto e = this->createScalarField();
            e->set(field, value);
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
        bool addMessageField(const char *field, const BaseMessage *msg);

        /**
         * Adds a date-time field to the message.
         *
         * @param field The field identifier name.
         * @param time Sets the field value to this time.
         * @return `true` if the field was successfully added, `false` otherwise
         */
        bool addDateTimeField(const char *field, const std::chrono::time_point<std::chrono::system_clock> &time);

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

        bool getDataField(const char *field, const char **value, size_t &length) const {
            if (field != nullptr) {
                auto index = m_keys.find(field);
                if (index != m_keys.end()) {
                    const DataFieldType *element = reinterpret_cast<DataFieldType *>(m_payload[index->second]);
                    length = element->get(value);
                    return true;
                }
            }
            return false;
        }

        /**
         * Detach the message from the library.
         * Passes the ownership of the message to the caller, it is now the responsibility
         * of the caller to free up the resources.
         */
        void detach() noexcept;

        /// @cond DEV

        // from Serializable
        virtual const size_t encode(MessageBuffer &buffer) const noexcept override;
        virtual const bool decode(const MessageBuffer::ByteStorageType &buffer) override;

        // from reusable
        void prepareForReuse() override {
            this->clear();
        }

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

#endif //TFDCF_MESSAGE_H
