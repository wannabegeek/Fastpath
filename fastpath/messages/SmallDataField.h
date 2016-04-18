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

#ifndef FASTPATH_SMALLDATAFIELD_H
#define FASTPATH_SMALLDATAFIELD_H

#include "fastpath/messages/DataField.h"

#include <cstring>

namespace fp {
    class SmallDataField final : public DataField {
    public:
        static constexpr std::size_t max_size = 256;
    private:
        byte m_storage[max_size];

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override {
            if (typeid(other) == typeid(SmallDataField)) {
                const SmallDataField &f = static_cast<const SmallDataField &>(other);
                return std::memcmp(m_storage, f.m_storage, m_data_length) == 0;
            }
            return false;
        }

        virtual std::ostream& output(std::ostream& out) const override {
            out << m_identifier << ":" << StorageTypeDescription[m_type] << "=";
            switch (m_type) {
                case storage_type::string: {
                    out << std::string(reinterpret_cast<const char *>(m_storage), m_data_length - 1); // -1 for NULL
                    break;
                }
                case storage_type::data: {
                    out << "[data of " << m_data_length << " bytes]";
                    break;
                }
                default:
                    // we can't handle any other message type
                    break;
            }

            return out;
        }

    public:
        SmallDataField(const char *identifier, const char *value) noexcept : DataField(identifier, storage_type::string, strlen(value) + 1) {
            std::strncpy(reinterpret_cast<char *>(m_storage), value, m_data_length);
        }

        SmallDataField(const char *identifier, const byte *value, const std::size_t length) noexcept : DataField(identifier, storage_type::data, length) {
            std::memcpy(m_storage, value, m_data_length);
        }

        SmallDataField(const MessageBuffer::ByteStorageType &buffer) : DataField(buffer) {
            std::memcpy(m_storage, buffer.readBytes(), m_data_length);
            buffer.advanceRead(m_data_length);
        }

        const size_t get(const byte **data) const noexcept override {
            assert(m_type == storage_type::data);
            *data = m_storage;
            return m_data_length;
        }

        const size_t get(const char **data) const noexcept override {
            assert(m_type == storage_type::string);
            *data = reinterpret_cast<const char *>(m_storage);
            return m_data_length;
        }

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_pointer<T>::value>> const T get() const noexcept {
            return DataField::get<T>();
        }

        const size_t encode(MessageBuffer &buffer) const noexcept override {
            return Field::encode(buffer, m_storage, m_data_length);
        }
    };
}

#endif //FASTPATH_SMALLDATAFIELD_H
