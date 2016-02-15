//
// Created by Tom Fewster on 12/02/2016.
//

#ifndef TFDCF_ENCODER_H
#define TFDCF_ENCODER_H

namespace DCF {
    class Encoder {
    public:
        virtual ~Encoder() {}

        virtual void encode() = 0;
    };
}

#endif //TFDCF_ENCODER_H
