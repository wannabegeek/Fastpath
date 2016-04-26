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

#ifndef FASTPATH_DATETIMEFIELD_H
#define FASTPATH_DATETIMEFIELD_H

#include <chrono>
#include "fastpath/messages/Field.h"

namespace fp {
    class DateTimeField final : public Field {
    private:
        static constexpr int fp_seconds = 0;
        static constexpr int fp_microseconds = 1;
        static constexpr std::size_t data_size = sizeof(int64_t) + sizeof(int64_t);

        int64_t m_time[2];

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override {
            if (typeid(other) == typeid(DateTimeField)) {
                const DateTimeField &f = static_cast<const DateTimeField &>(other);
                return m_time[fp_seconds] == f.m_time[fp_seconds]
                       && m_time[fp_microseconds] == f.m_time[fp_microseconds];
            }
            return false;
        }

        virtual std::ostream& output(std::ostream& out) const override {
            out << m_identifier << ":date_time=" << m_time[fp_seconds] << "s " << m_time[fp_microseconds] << "us";
            return out;
        }

    public:
        DateTimeField(const char *identifier, const uint64_t seconds, const uint64_t microseconds) noexcept : Field(identifier, storage_type::date_time, data_size) {
            m_time[fp_seconds] = seconds;
            m_time[fp_microseconds] = microseconds;
        }

        DateTimeField(const char *identifier, const std::chrono::time_point<std::chrono::system_clock> &value) noexcept : Field(identifier, storage_type::date_time, data_size) {
            auto time = std::chrono::duration_cast<std::chrono::microseconds>(value.time_since_epoch());
            m_time[fp_seconds] = std::chrono::duration_cast<std::chrono::seconds>(time).count();
            m_time[fp_microseconds] = std::chrono::duration_cast<std::chrono::microseconds>(time).count() % std::chrono::microseconds::period::den;
        }

        DateTimeField(const char *identifier, const std::chrono::microseconds &value) noexcept : Field(identifier, storage_type::date_time, data_size) {
            m_time[fp_seconds] = std::chrono::duration_cast<std::chrono::seconds>(value).count();
            m_time[fp_microseconds] = std::chrono::duration_cast<std::chrono::microseconds>(value).count() % std::chrono::microseconds::period::den;
        }

        DateTimeField(const MessageBuffer::ByteStorageType &buffer) throw(fp::exception) : Field(buffer) {
            m_time[fp_seconds] = readScalar<int64_t>(buffer.readBytes());
            buffer.advanceRead(sizeof(int64_t));
            m_time[fp_microseconds] = readScalar<int64_t>(buffer.readBytes());
            buffer.advanceRead(sizeof(int64_t));
        }

        const void get(std::chrono::time_point<std::chrono::system_clock> &data) const noexcept {
            std::chrono::microseconds ms((m_time[fp_seconds] * std::chrono::microseconds::period::den) + m_time[fp_microseconds]);
            std::chrono::time_point<std::chrono::system_clock, std::chrono::microseconds> time(ms);
            data = time;
        }

        const void get(uint64_t &seconds, uint64_t &microseconds) const noexcept {
            seconds = m_time[fp_seconds];
            microseconds = m_time[fp_microseconds];
        }

        const std::chrono::microseconds get() const noexcept {
            return std::chrono::microseconds((m_time[fp_seconds] * std::chrono::microseconds::period::den) + m_time[fp_microseconds]);
        }

        const size_t encode(MessageBuffer::MutableByteStorageType &buffer) const noexcept override {
            return Field::encode(buffer, reinterpret_cast<const byte *>(m_time), data_size);
        }
    };
}

#endif //FASTPATH_DATETIMEFIELD_H
