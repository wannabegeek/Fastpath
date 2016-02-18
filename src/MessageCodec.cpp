//
// Created by Tom Fewster on 18/02/2016.
//

#include "MessageCodec.h"
#include "MessageBuffer.h"
#include "Message.h"

namespace DCF {

    std::shared_ptr<Message> MessageCodec::decode(MessageBuffer &buffer) {
        if (buffer.size() > sizeof(MsgHeader)) {

        }

        return nullptr;
    }

    bool MessageCodec::encode(std::shared_ptr<Message> &msg, MessageBuffer &buffer) {
        byte *b = buffer.allocate(sizeof(MsgHeader));

        MsgHeader *header = reinterpret_cast<MsgHeader *>(b);
        header->msg_length = 256;
        header->flags = msg->flags();
        header->field_count = msg->size();
        header->subject_length = static_cast<uint16_t>(strlen(msg->subject()));
        buffer.append(reinterpret_cast<const byte *>(msg->subject()), header->subject_length);

        for (int i = 0; i < header->field_count; i++) {
            b = buffer.allocate(sizeof(Field));
            MsgField *field = reinterpret_cast<MsgField *>(b);
            field->identifier = 123;
            field->type = StorageType::string;
            field->data_length = 8;
            buffer.append(reinterpret_cast<const byte *>("ewqtewqe"), field->data_length);
        }
        return true;
    }
}