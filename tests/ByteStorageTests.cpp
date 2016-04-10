//
// Created by fewstert on 23/02/16.
//

#include <gtest/gtest.h>
#include <ByteStorage.h>
#include <types.h>

TEST(ByteStorage, StoreCopy) {
    const char *data = "Hello World";

    DCF::ByteStorage<byte> buffer(reinterpret_cast<const byte *>(data), strlen(data) + 1);

    ASSERT_EQ(strlen(data) + 1, buffer.length());
    ASSERT_TRUE(buffer.owns_copy());
    const byte *bytes = nullptr;
    ASSERT_EQ(strlen(data) + 1, buffer.bytes(&bytes));
    ASSERT_STREQ(data, reinterpret_cast<const char *>(bytes));
    ASSERT_NE(reinterpret_cast<const byte *>(data), bytes);
}

TEST(ByteStorage, StoreNoCopy) {
    const char *data = "Hello World";

    DCF::ByteStorage<byte> buffer(reinterpret_cast<const byte *>(data), strlen(data) + 1, true);

    ASSERT_EQ(strlen(data) + 1, buffer.length());
    ASSERT_FALSE(buffer.owns_copy());
    const byte *bytes = nullptr;
    ASSERT_EQ(strlen(data) + 1, buffer.bytes(&bytes));
    ASSERT_STREQ(data, reinterpret_cast<const char *>(bytes));
    ASSERT_EQ(reinterpret_cast<const byte *>(data), bytes);
}

TEST(ByteStorage, Operator) {
    const char *data = "Hello World";

    DCF::ByteStorage<byte> buffer(reinterpret_cast<const byte *>(data), strlen(data) + 1);

    for (size_t i = 0; i < strlen(data); i++) {
        ASSERT_EQ(data[i], buffer[i]);
    }
}