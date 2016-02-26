//
// Created by Tom Fewster on 12/02/2016.
//

#ifndef TFDCF_ENCODER_H
#define TFDCF_ENCODER_H

#include "MessageBuffer.h"

namespace DCF {
    class Encoder {
    public:
        virtual ~Encoder() {}

        virtual const size_t encode(MessageBuffer &buffer) noexcept = 0;
    };
}

#endif //TFDCF_ENCODER_H
