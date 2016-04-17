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

#ifndef TFDCF_LARGEDATAFIELD_H
#define TFDCF_LARGEDATAFIELD_H

#include "fastpath/messages/DataField.h"

namespace DCF {
    template <typename Allocator> class LargeDataField final : public DataField {
    private:
        MutableByteStorage<byte, Allocator> m_storage;

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override {
            if (typeid(other) == typeid(LargeDataField)) {
                const LargeDataField &f = static_cast<const LargeDataField &>(other);
                return m_type == f.m_type && m_storage == f.m_storage;
            }
            return false;
        }

        virtual std::ostream& output(std::ostream& out) const override {
            switch (m_type) {
                case StorageType::string: {
                    const byte *data;
                    const size_t size = m_storage.bytes(&data);
                    out << m_identifier << ":string=" << std::string(reinterpret_cast<const char *>(data), size - 1); // -1 for NULL
                    break;
                }
                case StorageType::data: {
                    out << m_identifier << ":opaque=" << "[data of " << m_storage.length() << " bytes]";
                    break;
                }
                default:
                    // we can't handle any other message type
                    break;
            }

            return out;
        }

    public:
        LargeDataField(const char *identifier, const char *value, const Allocator allocator = Allocator()) noexcept : DataField(identifier, StorageType::string, strlen(value) + 1), m_storage(512, allocator) {
            m_storage.setData(reinterpret_cast<const byte *>(value), m_data_length);
        }

        LargeDataField(const char *identifier, const byte *value, const std::size_t length, const Allocator allocator = Allocator()) noexcept : DataField(identifier, StorageType::data, length), m_storage(length, allocator) {
            m_storage.setData(reinterpret_cast<const byte *>(value), length);
        }

        LargeDataField(const MessageBuffer::ByteStorageType &buffer, const Allocator allocator = Allocator()) throw(fp::exception) : DataField(buffer), m_storage(512, allocator) {
            m_storage.setData(buffer.readBytes(), m_data_length);
            buffer.advanceRead(m_data_length);
        }

        const size_t get(const byte **data) const noexcept override {
            assert(m_type == StorageType::data);
            return m_storage.bytes(data);
        }

        const size_t get(const char **data) const noexcept override {
            assert(m_type == StorageType::string);
            const byte *bytes = nullptr;
            const size_t r =m_storage.bytes(&bytes);
            *data = reinterpret_cast<const char *>(bytes);
            return r;
        }

        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_pointer<T>::value>> const T get() const noexcept {
            return DataField::get<T>();
        }

        const size_t encode(MessageBuffer &buffer) const noexcept override {
            const byte *data = nullptr;
            const size_t data_len = m_storage.bytes(&data);
            return Field::encode(buffer, data, data_len);
        }
    };
}

#endif //TFDCF_LARGEDATAFIELD_H
