/***************************************************************************
                          __FILE__
                          -------------------
    copyright            : Copyright (c) 2004-2016 Tom Fewster
    email                : tom@wannabegeek.com
    date                 : 04/03/2016

 ***************************************************************************/

/***************************************************************************
 * This library is free software; you can redistribute it and/or           *
 * modify it under the terms of the GNU Lesser General Public              *
 * License as published by the Free Software Foundation; either            *
 * version 2.1 of the License, or (at your option) any later version.      *
 *                                                                         *
 * This library is distributed in the hope that it will be useful,         *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of          *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU       *
 * Lesser General Public License for more details.                         *
 *                                                                         *
 * You should have received a copy of the GNU Lesser General Public        *
 * License along with this library; if not, write to the Free Software     *
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA *
 ***************************************************************************/

#include <gtest/gtest.h>

#include "fastpath/MessageBuffer.h"

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

    const byte *data = nullptr;
    buffer.bytes(&data);
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], data[(i * 5) + j]);
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

    const byte *data = nullptr;
    buffer.bytes(&data);
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], data[(i * 5) + j]);
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

    const byte *data = nullptr;
    buffer.bytes(&data);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], data[(i * 5) + j]);
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

    const byte *data = nullptr;
    buffer1.bytes(&data);
    for (int i = 0; i < 8; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], data[(i * 5) + j]);
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
    const byte *data = nullptr;
    EXPECT_EQ(static_cast<decltype(buffer.bytes(&data))>(0), buffer.bytes(&data));
    EXPECT_EQ(nullptr, data);
}

TEST(MessageBuffer, EraseBackAll) {
    DCF::MessageBuffer buffer(16);

    const char *test = "01234567890123456789";
    buffer.append(reinterpret_cast<const byte *>(test), 20);

    const byte *orig = nullptr;
    buffer.bytes(&orig);

    buffer.erase_back(20);
    const byte *data = nullptr;
    EXPECT_EQ(static_cast<decltype(buffer.bytes(&data))>(0), buffer.bytes(&data));
    EXPECT_EQ(nullptr, data);

    buffer.append(reinterpret_cast<const byte *>(test), 20);
    EXPECT_EQ(static_cast<decltype(buffer.bytes(&data))>(20), buffer.bytes(&data));
    EXPECT_EQ(static_cast<decltype(buffer.bytes(&data))>(20), buffer.length());
}

TEST(MessageBuffer, EraseBackPartial) {
    DCF::MessageBuffer buffer(16);

    const char *test = "01234567890123456789";
    buffer.append(reinterpret_cast<const byte *>(test), 20);

    buffer.erase_back(10);
    const byte *data = nullptr;
    const size_t length = buffer.bytes(&data);
    EXPECT_EQ(10u, length);
    EXPECT_EQ(0, strncmp("0123456789", reinterpret_cast<const char *>(data), length));

    buffer.append(reinterpret_cast<const byte *>(test), 20);
    EXPECT_EQ(30u, buffer.bytes(&data));
    EXPECT_EQ(30u, buffer.length());
}

TEST(MessageBuffer, EraseFrontPartial) {
    DCF::MessageBuffer buffer(16);

    const char *test = "0123456789ABCDE";
    buffer.append(reinterpret_cast<const byte *>(test), 16);

    buffer.erase_front(4);
    const byte *data = nullptr;
    const size_t length = buffer.bytes(&data);
    EXPECT_EQ(12u, length);
    EXPECT_STREQ("456789ABCDE", reinterpret_cast<const char *>(data));

    buffer.append(reinterpret_cast<const byte *>(test), 16);
    EXPECT_EQ(28u, buffer.bytes(&data));
    EXPECT_EQ(28u, buffer.length());
}

TEST(MessageBuffer, EraseFrontAll) {
    DCF::MessageBuffer buffer(16);

    const char *test = "0123456789ABCDE";
    buffer.append(reinterpret_cast<const byte *>(test), 16);

    const byte *orig = nullptr;
    buffer.bytes(&orig);

    buffer.erase_front(16);
    const byte *data = nullptr;
    EXPECT_EQ(0u, buffer.bytes(&data));
    EXPECT_EQ(nullptr, data);

    buffer.append(reinterpret_cast<const byte *>(test), 16);
    EXPECT_EQ(16u, buffer.bytes(&data));
    EXPECT_EQ(16u, buffer.length());
}

TEST(MessageBuffer, MoveConstructor) {
    DCF::MessageBuffer buffer1(16);

    const char *test = "Hello";
    for (int i = 0; i < 4; i++) {
        buffer1.append(reinterpret_cast<const byte *>(test), 5);
    }

    DCF::MessageBuffer buffer2 = DCF::MessageBuffer(std::move(buffer1));

    const byte *data = nullptr;
    EXPECT_EQ(0u, buffer1.bytes(&data));
    EXPECT_EQ(nullptr, data);

    EXPECT_EQ(20u, buffer2.bytes(&data));
    ASSERT_NE(nullptr, data);

    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            EXPECT_EQ(test[j], data[(i * 5) + j]);
        }
    }

}
