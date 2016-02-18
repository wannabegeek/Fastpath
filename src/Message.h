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

    class Message : public Encoder, Decoder {
    private:
        typedef std::vector<std::shared_ptr<Field>> PayloadContainer;
        typedef std::array<uint16_t, std::numeric_limits<uint16_t>::max() - 1> PayloadMapper;
        typedef std::unordered_map<std::string, std::vector<uint16_t>> KeyMappingsContainer;

        uint32_t m_size;
        uint8_t m_flags;
        char m_subject[std::numeric_limits<uint16_t>::max()];

        static constexpr uint16_t _NO_FIELD = std::numeric_limits<uint16_t>::max();


        PayloadContainer m_payload;
        PayloadMapper m_mapper;
        uint16_t m_maxRef;

        KeyMappingsContainer m_keys;

        const uint16_t findIdentifierByName(const std::string &field, const size_t instance = 0) const noexcept;

        const bool refExists(const uint16_t &field) const noexcept;
        const uint16_t createRefForString(const std::string &field) noexcept;

    public:
        Message() : m_size(0), m_flags(-1), m_maxRef(0) {
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

        template <typename T> void addField(const uint16_t &field, const T &value) {
            if (refExists(field)) {
                ThrowException(TF::Exception, "Ref already exists in message");
            }
            m_maxRef = std::max(m_maxRef, field);

            m_mapper[field] = m_payload.size();

            std::shared_ptr<Field> e = std::make_shared<Field>();
            e->setValue(value);
            m_payload.emplace_back(e);
            m_size++;
        }

        void addField(const uint16_t &field, const byte *value, const size_t size);

        bool removeField(const uint16_t &field);

        template <typename T> bool getField(const uint16_t &field, T &value) const {
            if (field != _NO_FIELD && field <= m_maxRef) {
                const std::shared_ptr<Field> element = m_payload[m_mapper[field]];
                return element.get()->get(value);
            }
            return false;
        }

        template <typename T> void addField(const std::string &field, const T &value) {
            const uint16_t ref = createRefForString(field);
            this->addField(ref, value);
        }

        void addField(const std::string &field, const byte *value, const size_t size);

        template <typename T> bool getField(const std::string &field, T &value, const size_t instance = 0) const {
            return this->getField(findIdentifierByName(field, instance), value);
        }

        bool removeField(const std::string &field, const size_t instance = 0);

        friend std::ostream &operator<<(std::ostream &out, const Message &msg) {
            out << "[start_index: " << "This is my message";
            return out;
        }

        void detach() noexcept;

        // from Encoder
        void encode(MessageBuffer &buffer) noexcept override;

        // from Decoder
        void decode(MessageBuffer &buffer) noexcept override;
    };
}

#endif //TFDCF_MESSAGE_H
