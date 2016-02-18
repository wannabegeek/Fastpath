//
// Created by Tom Fewster on 14/02/2016.
//

#ifndef TFDCF_DECODER_H
#define TFDCF_DECODER_H

#include "MessageBuffer.h"

namespace DCF {
    class Decoder {
    public:
        virtual ~Decoder() {}

        virtual void decode(MessageBuffer &buffer) noexcept = 0;
    };
}

#endif //TFDCF_DECODER_H
