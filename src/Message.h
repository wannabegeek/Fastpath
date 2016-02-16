//
// Created by Tom Fewster on 12/02/2016.
//

#ifndef TFDCF_MESSAGE_H
#define TFDCF_MESSAGE_H

#include <cstdint>
#include <string>
#include <vector>
#include <map>
#include <array>
#include <limits>
#include "Encoder.h"
#include "Element.h"
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
        typedef std::array<std::shared_ptr<Element>, std::numeric_limits<uint16_t>::max() - 1> PayloadContainer;
        typedef std::map<std::string, std::vector<uint16_t>> KeyMappingsContainer;

        int m_flags;
        std::string m_msgSubject;

        static const uint16_t _NO_FIELD = std::numeric_limits<uint16_t>::max();


        PayloadContainer m_payload;
        uint16_t m_maxRef;

        KeyMappingsContainer m_keys;

        const uint16_t findIdentifierByName(const std::string &field, const size_t instance = 0) const {
            auto it = m_keys.find(field);
            if (it != m_keys.end()) {
                if (instance < it->second.size()) {
                    return it->second[instance];
                }
            }

            return _NO_FIELD;
        }

        const bool refExists(const short &field) const {
//            size_t t = m_payload.size();
            return m_payload[field] != nullptr;
        }

    public:
        Message() : m_maxRef(0) {
            m_payload.fill(nullptr);
        }

        virtual ~Message() {};

        template <typename T> void addField(const uint16_t &field, const T &value) {
            m_maxRef = std::max(m_maxRef, field);
            if (refExists(field)) {
                ThrowException(TF::Exception, "Ref already exists in message");
            }
            std::shared_ptr<Element> e = std::make_shared<Element>();
            e->setValue(value);
            m_payload[field] = e;
        }

        bool removeField(const uint16_t &field) {
            if (field != _NO_FIELD && field <= m_maxRef) {
                m_payload[field] = nullptr;
                return true;
            }
            return false;
        }

        template <typename T> bool getField(const uint16_t &field, T &value) const {
            if (field != _NO_FIELD && field <= m_maxRef) {
                const std::shared_ptr<Element> element = m_payload[field];
                return element.get()->get(value);
            }
            return false;
        }


        template <typename T> void addField(const std::string &field, const T &value) {
            const uint16_t ref = m_maxRef++;
            auto it = m_keys.find(field);
            if (it == m_keys.end()) {
                auto t = std::pair<KeyMappingsContainer::key_type, KeyMappingsContainer::value_type>(field, KeyMappingsContainer::value_type());
                m_keys.emplace(t);
            } else {
                m_keys[field].push_back(ref);
            }

            this->addField(ref, value);
        }

        template <typename T> bool getField(const std::string &field, T &value, const size_t instance = 0) const {
            return this->getField(findIdentifierByName(field, instance), value);
        }

        bool removeField(const std::string &field, const size_t instance = 0) {
            auto it = m_keys.find(field);
            if (it != m_keys.end()) {
                auto &list = it->second;
                if (instance < list.size()) {
                    list.erase(list.begin() + instance);
                    if (list.empty()) {
                        m_keys.erase(it);
                    }
                }
            }

            return this->removeField(findIdentifierByName(field, instance));
        }

        friend std::ostream &operator<<(std::ostream &out, const Message &msg) {
            out << "[start_index: " << "This is my message";
            return out;
        }

        void detach();

        // from Encoder
        void encode() override {};

        // from Decoder
        void decode() override {};
    };

//    template <> void Message::addField(const std::string &field, const std::string &value) {
//        const uint16_t ref = m_maxRef++;
//        auto it = m_keys.find(field);
//        if (it == m_keys.end()) {
//            auto t = std::pair<KeyMappingsContainer::key_type, KeyMappingsContainer::value_type>(field, KeyMappingsContainer::value_type());
//            m_keys.emplace(t);
//        } else {
//            m_keys[field].push_back(ref);
//        }
//
//        this->addField(ref, value);
//    }

}

#endif //TFDCF_MESSAGE_H
