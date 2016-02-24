//
// Created by fewstert on 23/02/16.
//

#include <gtest/gtest.h>
#include <MutableByteStorage.h>

TEST(MutableByteStorage, StoreCopyReplace) {
    const char *data = "Hello World";
    const char *data2 = "Hello Another World Hello Another World";

    DCF::MutableByteStorage buffer(reinterpret_cast<const byte *>(data), strlen(data) + 1);
    buffer.setData(reinterpret_cast<const byte *>(data2), strlen(data2) + 1);

    ASSERT_EQ(strlen(data2) + 1, buffer.length());
    ASSERT_TRUE(buffer.owns_copy());

    const byte *bytes = nullptr;
    ASSERT_EQ(strlen(data2) + 1, buffer.bytes(&bytes));
    ASSERT_STREQ(data2, reinterpret_cast<const char *>(bytes));
    ASSERT_NE(reinterpret_cast<const byte *>(data2), bytes);
}

TEST(MutableByteStorage, UpdateLength) {
    const char *data = "Hello World";

    DCF::MutableByteStorage buffer(reinterpret_cast<const byte *>(data), strlen(data) + 1);
    const size_t len = buffer.capacity();

    buffer.increaseLengthBy(1024);

    ASSERT_LE(1024 + len, buffer.capacity());

    ASSERT_EQ(strlen(data) + 1 + 1024, buffer.length());

    const byte *bytes = nullptr;
    ASSERT_EQ(strlen(data) + 1 + 1024, buffer.bytes(&bytes));
    ASSERT_NE(reinterpret_cast<const byte *>(data), bytes);
}

TEST(MutableByteStorage, Clear) {
    const char *data = "Hello World";

    DCF::MutableByteStorage buffer(reinterpret_cast<const byte *>(data), strlen(data) + 1);
    const size_t capacity = buffer.capacity();

    buffer.clear();

    ASSERT_EQ(0, buffer.length());
    ASSERT_EQ(capacity, buffer.capacity());

    const byte *bytes = nullptr;
    ASSERT_EQ(0, buffer.bytes(&bytes));
    ASSERT_EQ(nullptr, bytes);
}

TEST(MutableByteStorage, AppendString) {
    const char *data = "Hello World";
    const char *data2 = ", Goodby Universe";

    DCF::MutableByteStorage buffer(reinterpret_cast<const byte *>(data), strlen(data));

    buffer.append(reinterpret_cast<const byte *>(data2), strlen(data2) + 1);

    ASSERT_EQ(strlen(data) + strlen(data2) + 1, buffer.length());

    const byte *bytes = nullptr;
    ASSERT_EQ(strlen(data) + strlen(data2) + 1, buffer.bytes(&bytes));
    ASSERT_STREQ("Hello World, Goodby Universe", reinterpret_cast<const char *>(bytes));
}

TEST(MutableByteStorage, AppendObject) {
    const char *data = "Hello World";
    const char *data2 = ", Goodby Universe";

    DCF::MutableByteStorage buffer(reinterpret_cast<const byte *>(data), strlen(data));
    DCF::MutableByteStorage buffer2(reinterpret_cast<const byte *>(data2), strlen(data2) + 1);

    buffer.append(buffer2);

    ASSERT_EQ(strlen(data) + strlen(data2) + 1, buffer.length());

    const byte *bytes = nullptr;
    ASSERT_EQ(strlen(data) + strlen(data2) + 1, buffer.bytes(&bytes));
    ASSERT_STREQ("Hello World, Goodby Universe", reinterpret_cast<const char *>(bytes));
}
