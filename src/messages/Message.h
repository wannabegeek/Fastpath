//
// Created by Tom Fewster on 12/02/2016.
//

#ifndef TFDCF_MESSAGE_H
#define TFDCF_MESSAGE_H

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
#include "ScalarField.h"
#include "DataField.h"
#include "MessageField.h"
#include "Utils/tfpool.h"
#include "types.h"

/**
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

    class Message : public Serializable, public tf::reusable {
    private:
        typedef std::vector<std::shared_ptr<Field>> PayloadContainer;
        typedef std::unordered_map<const char *, const size_t, FieldIdentifierHash, FieldIdentifierComparitor> KeyMappingsContainer;

        uint8_t m_flags;
        bool m_hasAddressing;
        char *m_subject;

        static constexpr const uint8_t addressing_flag = 1;
        static constexpr const uint8_t body_flag = 2;

        PayloadContainer m_payload;
        KeyMappingsContainer m_keys;

        const uint16_t findIdentifierByName(const std::string &field, const size_t instance = 0) const noexcept;

        static const DataStorageType getStorageType(const StorageType type);

        const size_t encodeAddressing(MessageBuffer &buffer) noexcept;
        const void encodeMsgLength(MessageBuffer &buffer, const MsgAddressing::msg_length length) noexcept;
        const bool decodeAddressing(const ByteStorage &buffer) noexcept;

    public:
        Message() : m_flags(-1), m_hasAddressing(true) {
            m_subject = new char[std::numeric_limits<uint16_t>::max()];
            m_subject[0] = '\0';
        }

        Message(Message &&msg) noexcept;

        virtual ~Message() {
            delete [] m_subject;
        };

        Message(const Message &msg) = delete;
        Message& operator=(Message const&) = delete;

        const char *subject() const { return m_subject; }
        const bool setSubject(const char *subject);

        const uint32_t size() const noexcept { return m_payload.size(); }
        const uint8_t flags() const noexcept { return m_flags; }

        const StorageType storageType(const char *field) const {
            const std::shared_ptr<Field> element = m_payload[m_keys.at(field)];
            return element->type();
        }

        void clear();

        ////////////// ADD ///////////////
        template <typename T, typename = std::enable_if<field_traits<T>::value && std::is_arithmetic<T>::value>> bool addScalarField(const char *field, const T &value) {
            std::shared_ptr<ScalarField> e = std::make_shared<ScalarField>();
            e->set(field, value);
            auto result = m_keys.insert(std::make_pair(e->identifier(), m_payload.size()));
            if (result.second) {
                m_payload.emplace_back(e);
            }
            return result.second;
        }

        bool addDataField(const char *field, const char *value);
        bool addDataField(const char *field, const byte *value, const size_t size);

        bool addMessageField(const char *field, const MessageType &msg);

        bool addDateTimeField(const char *field, const std::chrono::time_point<std::chrono::system_clock> &time);

        ////////////// REMOVE ///////////////

        bool removeField(const char *field);


        ////////////// ACCESSOR ///////////////

        template <typename T> bool getScalarField(const char *field, T &value) const {
            if (field != nullptr) {
                auto index = m_keys.find(field);
                if (index != m_keys.end()) {
                    const std::shared_ptr<ScalarField> element = std::static_pointer_cast<ScalarField>(
                            m_payload[index->second]);
                    value = element.get()->get<T>();
                    return true;
                }
            }
            return false;
        }

        bool getDataField(const char *field, const char **value, size_t &length) const {
            if (field != nullptr) {
                auto index = m_keys.find(field);
                if (index != m_keys.end()) {
                    const std::shared_ptr<DataField> element = std::static_pointer_cast<DataField>(
                            m_payload[index->second]);
                    length = element.get()->get(value);
                    return true;
                }
            }
            return false;
        }

        /////

        void detach() noexcept;

        const bool operator==(const Message &other) const;

        // from Serializable
        const size_t encode(MessageBuffer &buffer) noexcept override;
        const bool decode(const ByteStorage &buffer) noexcept override;

        // from reusable
        void prepareForReuse() override {
            this->clear();
        }

        friend std::ostream &operator<<(std::ostream &out, const Message &msg) {
            if (msg.m_hasAddressing) {
                if (msg.m_subject[0] == '\0') {
                    out << "<no subject>=";
                } else {
                    out << msg.m_subject << "=";
                }
            }
            bool first = true;
            for (const std::shared_ptr<Field> &field : msg.m_payload) {
                if (!first) {
                    out << ", ";
                }
                out << "{" << *field.get() << "}";
                first = false;
            }

            return out;
        }
    };
}

#endif //TFDCF_MESSAGE_H
