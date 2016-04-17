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
#include "fastpath/ByteStorage.h"
#include "fastpath/types.h"

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