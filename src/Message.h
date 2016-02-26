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
#include "Encoder.h"
#include "Field.h"
#include "Decoder.h"
#include "Exception.h"
#include "ScalarField.h"
#include "DataField.h"
#include "MessageField.h"

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

    class Message : public Encoder, Decoder {
    private:
        typedef std::vector<std::shared_ptr<Field>> PayloadContainer;
        typedef std::array<uint16_t, std::numeric_limits<uint16_t>::max() - 1> PayloadMapper;
        typedef std::unordered_map<std::string, std::vector<uint16_t>> KeyMappingsContainer;

        uint32_t m_size;
        uint8_t m_flags;
        bool m_hasAddressing;
        char m_subject[std::numeric_limits<uint16_t>::max()];

        static constexpr uint16_t _NO_FIELD = std::numeric_limits<uint16_t>::max();
        static constexpr const uint8_t addressing_flag = 1;
        static constexpr const uint8_t body_flag = 2;

        PayloadContainer m_payload;
        PayloadMapper m_mapper;
        uint16_t m_maxRef;

        KeyMappingsContainer m_keys;

        const uint16_t findIdentifierByName(const std::string &field, const size_t instance = 0) const noexcept;

        const bool refExists(const uint16_t &field) const noexcept;
        const uint16_t createRefForString(const std::string &field) noexcept;

        static const DataStorageType getStorageType(const StorageType type);

        const size_t encodeAddressing(MessageBuffer &buffer) noexcept;
        const size_t decodeAddressing(const ByteStorage &buffer) noexcept;

    public:
        Message() : m_size(0), m_flags(-1), m_hasAddressing(true), m_maxRef(0) {
            m_mapper.fill(_NO_FIELD);
            m_subject[0] = '\0';
        }

        virtual ~Message() {};

        const char *subject() const { return m_subject; }

        const bool setSubject(const char *subject) {
            if (strlen(subject) < std::numeric_limits<uint16_t>::max()) {
                strcpy(&m_subject[0], subject);
                return true;
            }

            return false;
        }

        const uint32_t size() const noexcept { return m_size; }
        const uint8_t flags() const noexcept { return m_flags; }

        const StorageType storageType(const uint16_t &field) const {
            if (refExists(field)) {
                const std::shared_ptr<Field> element = m_payload[m_mapper[field]];
                return element->type();
            }

            return StorageType::unknown;
        }

        void clear();

        ////////////// ADD ///////////////
        template <typename T> void addScalarField(const uint16_t &field, const T &value) {
            if (refExists(field)) {
                ThrowException(TF::Exception, "Ref already exists in message");
            }
            m_maxRef = std::max(m_maxRef, field);
            m_mapper[field] = m_payload.size();

            std::shared_ptr<ScalarField> e = std::make_shared<ScalarField>();
            e->set(field, value);
            m_payload.emplace_back(e);
            m_size++;
        }

        void addDataField(const uint16_t &field, const char *value) {
            if (refExists(field)) {
                ThrowException(TF::Exception, "Ref already exists in message");
            }
            m_maxRef = std::max(m_maxRef, field);
            m_mapper[field] = m_payload.size();

            std::shared_ptr<DataField> e = std::make_shared<DataField>();
            e->set(field, value);
            m_payload.emplace_back(e);
            m_size++;
        }

        void addDataField(const uint16_t &field, const std::string &value) {
            this->addDataField(field, value.c_str());
        }

        void addDataField(const uint16_t &field, const byte *value, const size_t size);

        void addMessageField(const uint16_t &field, const MessageType &msg);

        void addMessageField(const MessageType &msg);

        /////

        template <typename T> void addScalarField(const std::string &field, const T &value) {
            const uint16_t ref = createRefForString(field);
            this->addScalarField(ref, value);
        }

        void addDataField(const std::string &field, const char *value) {
            const uint16_t ref = createRefForString(field);
            this->addDataField(ref, value);
        }

        void addDataField(const std::string &field, const std::string value) {
            const uint16_t ref = createRefForString(field);
            this->addDataField(ref, value.c_str());
        }

        void addDataField(const std::string &field, const byte *value, const size_t size);

        void addMessageField(const std::string &field, const MessageType &msg);

        ////////////// REMOVE ///////////////

        bool removeField(const uint16_t &field);

        /////

        bool removeField(const std::string &field, const size_t instance = 0);

        ////////////// ACCESSOR ///////////////

        template <typename T> bool getScalarField(const uint16_t &field, T &value) const {
            if (field != _NO_FIELD && field <= m_maxRef) {
                const std::shared_ptr<ScalarField> element = std::static_pointer_cast<ScalarField>(m_payload[m_mapper[field]]);
                value = element.get()->get<T>();
                return true;
            }
            return false;
        }

        bool getDataField(const uint16_t &field, const char **value, size_t &length) const {
            if (field != _NO_FIELD && field <= m_maxRef) {
                const std::shared_ptr<DataField> element = std::static_pointer_cast<DataField>(m_payload[m_mapper[field]]);
                length = element.get()->get(value);
                return true;
            }
            return false;
        }

        bool getDataField(const uint16_t &field, std::string &value) const {
            if (field != _NO_FIELD && field <= m_maxRef) {
                const char *result = nullptr;
                size_t length = 0;
                bool r = this->getDataField(field, &result, length);
                value = std::string(result, length);
                return r;
            }
            return false;
        }

        /////

        template <typename T> const bool getScalarField(const std::string &field, T &value, const size_t instance = 0) const {
            return this->getScalarField(findIdentifierByName(field, instance), value);
        }

        const bool getDataField(const std::string &field, const char **value, size_t &length, const size_t instance = 0) const {
            return this->getDataField(findIdentifierByName(field, instance), value, length);
        }

        bool getDataField(const std::string &field, std::string &value, const size_t instance = 0) const {
            const char *result = nullptr;
            size_t length = 0;
            bool r = this->getDataField(findIdentifierByName(field, instance), &result, length);
            value = std::string(result, length);
            return r;
        }


        void detach() noexcept;

        // from Encoder
        const size_t encode(MessageBuffer &buffer) noexcept override;

        // from Decoder
        const size_t decode(const ByteStorage &buffer) noexcept override;

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
