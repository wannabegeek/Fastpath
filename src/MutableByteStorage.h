//
// Created by Tom Fewster on 17/02/2016.
//

#ifndef TFDCF_MUTABLEBYTESTORAGE_H
#define TFDCF_MUTABLEBYTESTORAGE_H


#include <stddef.h>
#include <utility>
#include <limits>
#include "types.h"
#include "ByteStorage.h"

namespace DCF {
    class MutableByteStorage : public ByteStorage {
    public:
        explicit MutableByteStorage(const size_t allocation = 256) noexcept;
        explicit MutableByteStorage(const byte *bytes, size_t length) noexcept;

        MutableByteStorage(MutableByteStorage &&orig) noexcept;

        virtual ~MutableByteStorage() noexcept {};

        void setData(const byte *data, const size_t length) noexcept;

        void increaseLengthBy(const size_t length) noexcept;

        const size_t capacity() const noexcept;
        void truncate(const size_t length) noexcept;

        byte *mutableBytes() const noexcept;

        void append(const byte *buffer, const size_t length) noexcept;
        void append(const ByteStorage &src, const size_t length = std::numeric_limits<size_t>::max()) noexcept;

        void clear() noexcept;

    };
}

#endif //TFDCF_MUTABLEBYTESTORAGE_H
