//
// Created by Tom Fewster on 23/02/2016.
//

#ifndef TFDCF_BYTESTORAGE_H
#define TFDCF_BYTESTORAGE_H

#include "Types.h"

namespace DCF {
    class ByteStorage {
    protected:
        using BufferDataType = std::pair<byte *, size_t>;

        BufferDataType m_storage;
        size_t m_storedLength;

        bool m_no_copy;

        void allocateStorage(const size_t length);

        explicit ByteStorage();

    public:
        explicit ByteStorage(byte *bytes, size_t length, bool nocopy=false);
        virtual ~ByteStorage();

        const size_t bytes(const byte **data) const;
        const size_t length() const { return m_storedLength; }

        const bool owns_copy() const { return !m_no_copy; }

        const byte operator[](const size_t index) const {
            return m_storage.first[index];
        }
    };
}

#endif //TFDCF_BYTESTORAGE_H
