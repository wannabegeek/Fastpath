//
// Created by Tom Fewster on 17/02/2016.
//

#ifndef TFDCF_MUTABLEBYTESTORAGE_H
#define TFDCF_MUTABLEBYTESTORAGE_H


#include <stddef.h>
#include <utility>
#include "Types.h"
#include "ByteStorage.h"

namespace DCF {
    class MutableByteStorage : public ByteStorage {
    public:
        explicit MutableByteStorage();
        explicit MutableByteStorage(byte *bytes, size_t length);

        virtual ~MutableByteStorage() {};

        void setData(const byte *data, const size_t length);

        void increaseLengthBy(const size_t length) noexcept;

        void append(const byte *buffer, const size_t length) noexcept;
        void append(const ByteStorage &src, const size_t length = 0) noexcept;

        void clear() { m_storedLength = 0; }

    };
}

#endif //TFDCF_MUTABLEBYTESTORAGE_H
