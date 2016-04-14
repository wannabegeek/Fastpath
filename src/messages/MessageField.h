//
// Created by Tom Fewster on 25/02/2016.
//

#ifndef TFDCF_MESSAGEFIELD_H
#define TFDCF_MESSAGEFIELD_H

#include "Field.h"
#include "BaseMessage.h"

namespace DCF {
    class MessageField final : public Field {
    private:
        BaseMessage m_msg;

    protected:
        virtual const bool isEqual(const Field &other) const override {
            if (typeid(other) == typeid(MessageField)) {
                const MessageField &f = static_cast<const MessageField &>(other);
                return m_msg == f.m_msg;
            }
            return false;
        }

        virtual std::ostream& output(std::ostream& out) const override;

    public:
//        MessageField(const char *identifier, const BaseMessage message) noexcept;
        MessageField(const char *identifier, BaseMessage &&message) noexcept;
        MessageField(const MessageBuffer::ByteStorageType &buffer) noexcept;

        const StorageType type() const noexcept override { return StorageType::message; }
        const size_t size() const noexcept override { return 0; }

        void set(const char *identifier, const BaseMessage msg);
        void set(const char *identifier, BaseMessage &&msg);
        const BaseMessage *get() const;

        const size_t encode(MessageBuffer &buffer) const noexcept override;
        const bool decode(const MessageBuffer::ByteStorageType &buffer) noexcept override;
    };
}
#endif //TFDCF_MESSAGEFIELD_H
