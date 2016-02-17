//
// Created by Tom Fewster on 17/02/2016.
//

#ifndef TFDCF_BYTESTORAGE_H
#define TFDCF_BYTESTORAGE_H


#include <stddef.h>
#include <utility>
#include "Types.h"

namespace DCF {
    class ByteStorage {
    private:
        using BufferDataType = std::pair<byte *, size_t>;

        BufferDataType m_storage;
        size_t m_storedLength;

        void resizeBuffer(const size_t length);

    public:
        explicit ByteStorage();

        ~ByteStorage();

        void storeData(const byte *data, const size_t length);

        const size_t retreiveData(const byte **data) const;

        const size_t size() const { return m_storedLength; }

        void clear() { m_storedLength = 0; }
    };
}

#endif //TFDCF_BYTESTORAGE_H
