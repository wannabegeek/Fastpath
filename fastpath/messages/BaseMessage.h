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
#include "fastpath/Exception.h"
#include "fastpath/utils/stringhash.h"
#include "fastpath/utils/tfpool.h"
#include "fastpath/utils/fast_linear_allocator.h"
#include "fastpath/types.h"
#include "fastpath/messages/StorageTypes.h"

namespace fp {
    class Field;

    /**
     * Base class containing the body implementation of a message.
     *
     * You shouldn't use this class directly you should use it derived class Message
     */
    class BaseMessage : public tf::reusable {
        friend class MessageCodec;

    protected:
        /// @cond DEV
        /**
         * Helper method for operator<<
         */
        virtual std::ostream& output(std::ostream& out) const;

        using field_allocator_type = tf::linear_allocator<unsigned char>;
        field_allocator_type::arena_type m_arena;
        field_allocator_type m_field_allocator;

        using payload_type = Field *;
        typedef std::vector<payload_type> PayloadContainer;
        typedef std::unordered_map<const char *, const size_t, tf::string_hash, tf::string_comparator> KeyMappingsContainer;

        PayloadContainer m_payload;
        KeyMappingsContainer m_keys;

        /// @endcond

    public:
        /**
         * Constructor
         */
        BaseMessage() noexcept : m_arena(8192), m_field_allocator(m_arena) {
            m_payload.reserve(64);
            m_keys.reserve(64);
        }

        /**
         * Move constructor
         */
        BaseMessage(BaseMessage &&msg) noexcept;
        virtual ~BaseMessage() noexcept;

        BaseMessage(const BaseMessage &msg) = delete;
        BaseMessage& operator=(BaseMessage const&) = delete;
        const bool operator==(const BaseMessage &other) const;
        /**
         * Return the number of fields contained in the message.
         *
         * @return number of fields.
         */
        inline const uint32_t size() const noexcept { return static_cast<uint32_t>(m_payload.size()); }

        /**
         * Return the storage type for a field
         *
         * @param field The field identifier name.
         * @return The storage type.
         */
        const storage_type storageType(const char *field) const noexcept;

        /**
         * Clears all the fields of the message, so it can be re-used.
         */
        virtual void clear() noexcept;

        ////////////// ACCESSOR ///////////////

        bool getScalarField(const char *field, bool &value) const noexcept;
        bool getScalarField(const char *field, int8_t &value) const noexcept;
        bool getScalarField(const char *field, int16_t &value) const noexcept;
        bool getScalarField(const char *field, int32_t &value) const noexcept;
        bool getScalarField(const char *field, int64_t &value) const noexcept;
        bool getScalarField(const char *field, uint8_t &value) const noexcept;
        bool getScalarField(const char *field, uint16_t &value) const noexcept;
        bool getScalarField(const char *field, uint32_t &value) const noexcept;
        bool getScalarField(const char *field, uint64_t &value) const noexcept;
        bool getScalarField(const char *field, float32_t &value) const noexcept;
        bool getScalarField(const char *field, float64_t &value) const noexcept;

        bool getDataField(const char *field, const char **value, size_t &length) const;

        /**
         * Detach the message from the library.
         * Passes the ownership of the message to the caller, it is now the responsibility
         * of the caller to free up the resources.
         */
        void detach() noexcept;

        /// @cond DEV

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

