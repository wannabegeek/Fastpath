//
// Created by fewstert on 29/02/16.
//

#ifndef TFDCF_DATETIMEFIELD_H
#define TFDCF_DATETIMEFIELD_H

#include <chrono>
#include "Field.h"

namespace DCF {
    class DateTimeField final : public Field {
    private:
        static constexpr int seconds = 0;
        static constexpr int microseconds = 1;

        std::chrono::time_point<std::chrono::system_clock> m_time_point;
        int64_t m_time[2];

        static constexpr std::size_t data_size = sizeof(int64_t) + sizeof(int64_t);

    protected:
        virtual const bool isEqual(const Field &other) const noexcept override {
            if (typeid(other) == typeid(DateTimeField)) {
                const DateTimeField &f = static_cast<const DateTimeField &>(other);
                return m_time[seconds] == f.m_time[seconds] && m_time[microseconds] == f.m_time[microseconds];
            }
            return false;
        }

        virtual std::ostream& output(std::ostream& out) const override {
            out << m_identifier << ":date_time=";

            return out;
        }

    public:
        DateTimeField(const char *identifier, const std::chrono::time_point<std::chrono::system_clock> &value) noexcept {
            setIdentifier(identifier);
            m_time_point = value;
            auto time = std::chrono::duration_cast<std::chrono::microseconds>(value.time_since_epoch());
            m_time[seconds] = std::chrono::duration_cast<std::chrono::seconds>(time).count();
            m_time[microseconds] = std::chrono::duration_cast<std::chrono::microseconds>(time).count() % 1000;
        }

        DateTimeField(const MessageBuffer::ByteStorageType &buffer) {
            if (buffer.remainingReadLength() >= MsgField::size()) {
                assert(static_cast<StorageType>(readScalar<MsgField::type>(buffer.readBytes())) == StorageType::date_time);
                buffer.advanceRead(sizeof(MsgField::type));

                const size_t identifier_length = readScalar<MsgField::identifier_length>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgField::identifier_length) + sizeof(MsgField::data_length));

                if (buffer.remainingReadLength() >= identifier_length) {
                    std::copy(buffer.readBytes(), &buffer.readBytes()[identifier_length], m_identifier);
                    m_identifier[identifier_length] = '\0';
                    buffer.advanceRead(identifier_length);
                    if (buffer.remainingReadLength() >= data_size) {
                        m_time[seconds] = readScalar<int64_t>(buffer.readBytes());
                        buffer.advanceRead(sizeof(int64_t));
                        m_time[microseconds] = readScalar<int64_t>(buffer.readBytes());
                        buffer.advanceRead(sizeof(int64_t));
                        return; // true;
                    }
                }
            }

            return; // false
        }

        const StorageType type() const noexcept override { return StorageType::date_time; }
        const size_t size() const noexcept override { return data_size; }

//        void set(const char *identifier, const std::chrono::time_point<std::chrono::system_clock> &value) {
//            setIdentifier(identifier);
//            m_time_point = value;
//            auto time = std::chrono::duration_cast<std::chrono::microseconds>(value.time_since_epoch());
//            m_time[seconds] = std::chrono::duration_cast<std::chrono::seconds>(time).count();
//            m_time[microseconds] = std::chrono::duration_cast<std::chrono::microseconds>(time).count() % 1000;
//        }

        const void get(std::chrono::time_point<std::chrono::system_clock> &data) const noexcept {
            data = m_time_point;
        }

        const std::chrono::time_point<std::chrono::system_clock> &get() const noexcept {
            return m_time_point;
        }

        const size_t encode(MessageBuffer &buffer) const noexcept override {
            return Field::encode(buffer, reinterpret_cast<const byte *>(m_time), data_size);
        }

        const bool decode(const MessageBuffer::ByteStorageType &buffer) noexcept override {
            if (buffer.remainingReadLength() >= MsgField::size()) {
                assert(static_cast<StorageType>(readScalar<MsgField::type>(buffer.readBytes())) == StorageType::date_time);
                buffer.advanceRead(sizeof(MsgField::type));

                const size_t identifier_length = readScalar<MsgField::identifier_length>(buffer.readBytes());
                buffer.advanceRead(sizeof(MsgField::identifier_length) + sizeof(MsgField::data_length));

                if (buffer.remainingReadLength() >= identifier_length) {
                    std::copy(buffer.readBytes(), &buffer.readBytes()[identifier_length], m_identifier);
                    m_identifier[identifier_length] = '\0';
                    buffer.advanceRead(identifier_length);
                    if (buffer.remainingReadLength() >= data_size) {
                        m_time[seconds] = readScalar<int64_t>(buffer.readBytes());
                        buffer.advanceRead(sizeof(int64_t));
                        m_time[microseconds] = readScalar<int64_t>(buffer.readBytes());
                        buffer.advanceRead(sizeof(int64_t));
                        return true;
                    }
                }
            }

            return false;
        }
    };
}

#endif //TFDCF_DATETIMEFIELD_H
