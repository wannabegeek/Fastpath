//
// Created by Tom Fewster on 25/02/2016.
//

#ifndef TFDCF_MESSAGEFIELD_H
#define TFDCF_MESSAGEFIELD_H

#include "Field.h"
#include "BaseMessage.h"

namespace DCF {
    class MessageField: public Field {
    private:
        const BaseMessage *m_msg;

    protected:
        virtual const bool isEqual(const Field &other) const override {
            try {
                const MessageField &f = dynamic_cast<const MessageField &>(other);
                return m_msg == f.m_msg;
            } catch (const std::bad_cast &e) {
                return false;
            }
        }

        virtual std::ostream& output(std::ostream& out) const override;

    public:
        const StorageType type() const noexcept override { return StorageType::message; }
        const size_t size() const noexcept override { return 0; }

        void set(const char *identifier, const BaseMessage *msg);
        const BaseMessage *get() const;

        const size_t encode(MessageBuffer &buffer) const noexcept override {
//            byte *b = buffer.allocate(sizeof(MsgField));
//            MsgField *field = reinterpret_cast<MsgField *>(b);
//            field->identifier = m_identifier;
//            field->type = m_type;
//            const byte *data = nullptr;
//            field->data_length = m_storage.bytes(&data);
//            buffer.append(data, field->data_length);
//
//            return sizeof(MsgField) + field->data_length;
            return 0;
        }

        const bool decode(const ByteStorage &buffer) noexcept override {
//            assert(buffer.length() > FieldHeaderSize());
//            const byte *data = nullptr;
//            buffer.bytes(&data);
//            const MsgField *field = reinterpret_cast<const MsgField *>(data);
//
//            m_identifier = field->identifier;
//            m_type = static_cast<StorageType>(field->type);
//            const size_t size = field->data_length;
//
//            assert(buffer.length() > FieldHeaderSize() + size);
//            m_storage.setData(&data[FieldHeaderSize()], size);
//
//            return FieldHeaderSize() + size;
            return 0;
        }
    };
}
#endif //TFDCF_MESSAGEFIELD_H
