#include <gtest/gtest.h>

#include "MessageBuffer.h"

TEST(MessageBuffer, SimpleAllocation) {
    DCF::MessageBuffer buffer(10);

    const char *test = "Hello";
    for (int i = 0; i < 2; i++) {
        buffer.append(reinterpret_cast<const byte *>(test), 5);
    }

    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], buffer[(i * 5) + j]);
        }
    }

    const DCF::MessageBuffer::BufferDataType data = buffer.data();
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], data.first[(i * 5) + j]);
        }
    }
}

TEST(MessageBuffer, SingleAllocation) {
    DCF::MessageBuffer buffer(16);

    const char *test = "Hello";
    for (int i = 0; i < 4; i++) {
        buffer.append(reinterpret_cast<const byte *>(test), 5);
    }

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], buffer[(i * 5) + j]);
        }
    }

    const DCF::MessageBuffer::BufferDataType data = buffer.data();
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], data.first[(i * 5) + j]);
        }
    }
}

TEST(MessageBuffer, DoubleAllocation) {
    DCF::MessageBuffer buffer(16);

    const char *test = "Hello";
    for (int i = 0; i < 8; i++) {
        buffer.append(reinterpret_cast<const byte *>(test), 5);
    }

    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], buffer[(i * 5) + j]);
        }
    }

    const DCF::MessageBuffer::BufferDataType data = buffer.data();
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], data.first[(i * 5) + j]);
        }
    }
}

TEST(MessageBuffer, Append) {
    DCF::MessageBuffer buffer1(16);
    DCF::MessageBuffer buffer2(16);

    const char *test = "Hello";
    for (int i = 0; i < 4; i++) {
        buffer1.append(reinterpret_cast<const byte *>(test), 5);
    }
    for (int i = 0; i < 4; i++) {
        buffer2.append(reinterpret_cast<const byte *>(test), 5);
    }

    buffer1.append(buffer2);

    const DCF::MessageBuffer::BufferDataType data = buffer1.data();
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], data.first[(i * 5) + j]);
        }
    }
}

TEST(MessageBuffer, Clear) {
    DCF::MessageBuffer buffer(16);

    const char *test = "Hello";
    for (int i = 0; i < 4; i++) {
        buffer.append(reinterpret_cast<const byte *>(test), 5);
    }

    buffer.clear();
    const DCF::MessageBuffer::BufferDataType data = buffer.data();
    EXPECT_EQ(static_cast<decltype(data.second)>(0), data.second);
}

TEST(MessageBuffer, EraseBackAll) {
    DCF::MessageBuffer buffer(16);

    const char *test = "01234567890123456789";
    buffer.append(reinterpret_cast<const byte *>(test), 20);

    const byte *orig = buffer.data().first;
    buffer.erase_back(20);
    const DCF::MessageBuffer::BufferDataType data = buffer.data();
    EXPECT_EQ(static_cast<decltype(data.second)>(0), data.second);
    EXPECT_EQ(orig, data.first);

    buffer.append(reinterpret_cast<const byte *>(test), 20);
    EXPECT_EQ(static_cast<decltype(buffer.data().second)>(20), buffer.data().second);
    EXPECT_EQ(static_cast<decltype(buffer.data().second)>(20), buffer.size());
}

TEST(MessageBuffer, EraseBackPartial) {
    DCF::MessageBuffer buffer(16);

    const char *test = "01234567890123456789";
    buffer.append(reinterpret_cast<const byte *>(test), 20);

    buffer.erase_back(10);
    const DCF::MessageBuffer::BufferDataType data = buffer.data();
    EXPECT_EQ(static_cast<decltype(data.second)>(10), data.second);
    EXPECT_EQ(0, strncmp("0123456789", reinterpret_cast<const char *>(data.first), data.second));

    buffer.append(reinterpret_cast<const byte *>(test), 20);
    EXPECT_EQ(static_cast<decltype(buffer.data().second)>(30), buffer.data().second);
    EXPECT_EQ(static_cast<decltype(buffer.data().second)>(30), buffer.size());
}

TEST(MessageBuffer, EraseFrontPartial) {
    DCF::MessageBuffer buffer(16);

    const char *test = "0123456789ABCDE";
    buffer.append(reinterpret_cast<const byte *>(test), 16);

    buffer.erase_front(4);
    const DCF::MessageBuffer::BufferDataType data = buffer.data();
    EXPECT_EQ(static_cast<decltype(data.second)>(12), data.second);
    EXPECT_STREQ("456789ABCDE", reinterpret_cast<const char *>(data.first));

    buffer.append(reinterpret_cast<const byte *>(test), 16);
    EXPECT_EQ(static_cast<decltype(buffer.data().second)>(28), buffer.data().second);
    EXPECT_EQ(static_cast<decltype(buffer.data().second)>(28), buffer.size());
}

TEST(MessageBuffer, EraseFrontAll) {
    DCF::MessageBuffer buffer(16);

    const char *test = "0123456789ABCDE";
    buffer.append(reinterpret_cast<const byte *>(test), 16);

    const byte *orig = buffer.data().first;
    buffer.erase_front(16);
    const DCF::MessageBuffer::BufferDataType data = buffer.data();
    EXPECT_EQ(static_cast<decltype(data.second)>(0), data.second);
    EXPECT_EQ(orig, data.first);

    buffer.append(reinterpret_cast<const byte *>(test), 16);
    EXPECT_EQ(static_cast<decltype(buffer.data().second)>(16), buffer.data().second);
    EXPECT_EQ(static_cast<decltype(buffer.data().second)>(16), buffer.size());
}

TEST(MessageBuffer, MoveConstructor) {
    DCF::MessageBuffer buffer1(16);

    const char *test = "Hello";
    for (int i = 0; i < 4; i++) {
        buffer1.append(reinterpret_cast<const byte *>(test), 5);
    }

    DCF::MessageBuffer buffer2 = DCF::MessageBuffer(std::move(buffer1));

    const DCF::MessageBuffer::BufferDataType data1 = buffer1.data();
    EXPECT_NE(nullptr, data1.first);
    EXPECT_EQ(static_cast<decltype(data1.second)>(0), data1.second);

    const DCF::MessageBuffer::BufferDataType data2 = buffer2.data();
    ASSERT_NE(nullptr, data1.first);
    EXPECT_EQ(static_cast<decltype(data2.second)>(20), data2.second);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], data2.first[(i * 5) + j]);
        }
    }

}
