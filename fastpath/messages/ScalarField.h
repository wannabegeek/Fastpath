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

#ifndef FASTPATH_SCALARFIELD_H
#define FASTPATH_SCALARFIELD_H

#include <stddef.h>
#include <string>
#include <type_traits>
#include <iostream>
#include <cassert>
#include <algorithm>

#include "fastpath/messages/StorageTypes.h"
#include "fastpath/messages/FieldTraits.h"
#include "fastpath/messages/Field.h"

namespace fp {

    class ScalarField final : public Field {

    private:
        byte m_raw[sizeof(float64_t)] = {0};

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override {
            if (typeid(other) == typeid(ScalarField)) {
                const ScalarField &f = static_cast<const ScalarField &>(other);
                return std::equal(std::begin(m_raw), std::end(m_raw), std::begin(f.m_raw));
            }
            return false;
        }

        virtual std::ostream& output(std::ostream& out) const override {
            out << m_identifier << ":" << StorageTypeDescription[m_type] << "=";
            switch (m_type) {
                case storage_type::boolean:
                    out << std::boolalpha << get<bool>();
                    break;
                case storage_type::uint8:
                    out << get<uint8_t>();
                    break;
                case storage_type::uint16:
                    out << get<uint16_t>();
                    break;
                case storage_type::uint32:
                    out << get<uint32_t>();
                    break;
                case storage_type::uint64:
                    out << get<uint64_t>();
                    break;
                case storage_type::int8:
                    out << get<int8_t>();
                    break;
                case storage_type::int16:
                    out << get<int16_t>();
                    break;
                case storage_type::int32:
                    out << get<int32_t>();
                    break;
                case storage_type::int64:
                    out << get<int64_t>();
                    break;
                case storage_type::float32:
                    out << get<float32_t>();
                    break;
                case storage_type::float64:
                    out << get<float64_t>();
                    break;
                default:
                    // we can't handle any other message type
                    break;
            }

            return out;
        }

    public:
        template<typename T, typename = std::enable_if<field_traits<T>::value && std::is_arithmetic<T>::value>> ScalarField(const char *identifier, const T &value) noexcept : Field(identifier, field_traits<T>::type, field_traits<T>::size) {
            writeScalar(m_raw, value);
        }

        ScalarField(const MessageBuffer::ByteStorageType &buffer) throw(fp::exception) : Field(buffer) {
            std::copy(buffer.readBytes(), &buffer.readBytes()[m_data_length], m_raw);
            buffer.advanceRead(m_data_length);
        }

        ScalarField(const ScalarField &) = delete;

        template<typename T, typename = std::enable_if<field_traits<T>::value && std::is_arithmetic<T>::value>>
        const T get() const noexcept {
            assert(m_type == field_traits<T>::type);
            return readScalar<T>(m_raw);
        }

        const size_t encode(MessageBuffer &buffer) const noexcept override {
            return Field::encode(buffer, reinterpret_cast<const byte *>(m_raw), m_data_length);
        }
    };
}

#endif //FASTPATH_SCALARFIELD_H
