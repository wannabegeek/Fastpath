//
// Created by Tom Fewster on 18/02/2016.
//

#ifndef TFDCF_MESSAGECODEC_H
#define TFDCF_MESSAGECODEC_H

#include <memory>

namespace DCF {
    class Message;
    class MessageBuffer;

    class MessageCodec {
    public:
        static std::shared_ptr<Message> decode(MessageBuffer &buffer);
        static bool encode(std::shared_ptr<Message> &msg, MessageBuffer &buffer);
    };
}

#endif //TFDCF_MESSAGECODEC_H
