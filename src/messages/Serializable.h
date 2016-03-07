//
// Created by Tom Fewster on 26/02/2016.
//

#ifndef TFDCF_SERIALIZABLE_H
#define TFDCF_SERIALIZABLE_H

#include "types.h"

namespace DCF {
    template<typename T> T endianScalar(T t) {
#ifdef __BIG_ENDIAN__
        if (sizeof(T) == 1) {   // Compile-time if-then's.
            return t;
        } else if (sizeof(T) == 2) {
            auto r = __builtin_bswap16(*reinterpret_cast<uint16_t *>(&t));
            return *reinterpret_cast<T *>(&r);
        } else if (sizeof(T) == 4) {
            auto r = __builtin_bswap32(*reinterpret_cast<uint32_t *>(&t));
            return *reinterpret_cast<T *>(&r);
        } else if (sizeof(T) == 8) {
            auto r = __builtin_bswap64(*reinterpret_cast<uint64_t *>(&t));
            return *reinterpret_cast<T *>(&r);
        } else {
            assert(0);
        }
#else
        return t;
#endif
    }

    template<typename T> T readScalar(const byte *p) {
        return endianScalar(*reinterpret_cast<const T *>(p));
    }

    template<typename T> byte *writeScalar(byte *p, T t) {
        *reinterpret_cast<T *>(p) = endianScalar(t);
        return p + sizeof(T);
    }

    class MessageBuffer;
    class ByteStorage;

    class Serializable {
    public:
        virtual ~Serializable() {}

        virtual const size_t encode(MessageBuffer &buffer) const noexcept = 0;
        virtual const bool decode(const ByteStorage &buffer) noexcept = 0;
    };

}

#endif //TFDCF_SERIALIZABLE_H
