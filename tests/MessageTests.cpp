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
#include "fastpath/messages/MutableMessage.h"
#include "fastpath/utils/logger.h"

TEST(Message, SetSubject) {
    fp::MutableMessage msg;
    const char *subject = "TEST.SUBJECT";
    ASSERT_TRUE(msg.setSubject(subject));

    ASSERT_STREQ(subject, msg.subject());

    ASSERT_TRUE(msg.setSubject("UPDATED.SUBJECT"));
    ASSERT_STRNE(subject, msg.subject());
}

TEST(Message, AddStringField) {

    fp::MutableMessage msg;
    ASSERT_TRUE(msg.addDataField("1234", "TEST"));

    const char * v = nullptr;
    size_t len = 0;
    EXPECT_TRUE(msg.getDataField("1234", &v, len));
    EXPECT_EQ(5u, len);
    EXPECT_STREQ("TEST", v);

    msg.addDataField("TEST", "AGAIN");
    ASSERT_TRUE(msg.getDataField("TEST", &v, len));
    ASSERT_EQ(6u, len);
    ASSERT_STREQ("AGAIN", v);
}

TEST(Message, AddFloatField) {
    fp::MutableMessage msg;
    msg.addScalarField("TEST", static_cast<float32_t>(1.4));
    float32_t t = 0.0;
    ASSERT_TRUE(msg.getScalarField("TEST", t));
    ASSERT_FLOAT_EQ(1.4, t);
}

TEST(Message, AddMixedDuplicateField) {
    fp::MutableMessage msg;
    msg.addDataField("TEST", "AGAIN");

    ASSERT_FALSE(msg.addScalarField("TEST", static_cast<float32_t>(1.4)));
    ASSERT_EQ(fp::storage_type::string, msg.storageType("TEST"));
    const char *t = nullptr;
    size_t length = 0;
    ASSERT_TRUE(msg.getDataField("TEST", &t, length));
    ASSERT_STREQ("AGAIN", t);

}

TEST(Message, AddMessageField) {
    fp::MutableMessage msg;
    msg.addScalarField("TEST", static_cast<float32_t>(1.4));

    fp::MutableMessage m;
    m.addDataField("TEST2", "TOMTOMTOM");

    msg.addMessageField("MSG_TEST", std::move(m));

    DEBUG_LOG("Embedded msg: " << msg);
}

TEST(Message, RemoveFieldByString) {
    fp::MutableMessage msg;
    float32_t t = 22.0;
    msg.addScalarField("TEST", t);
    ASSERT_TRUE(msg.getScalarField("TEST", t));
    ASSERT_FLOAT_EQ(22, t);

    ASSERT_EQ(1u, msg.size());

    ASSERT_TRUE(msg.removeField("TEST"));
    ASSERT_EQ(0u, msg.size());
}

TEST(Message, Encode) {
    fp::MutableMessage msg;
    msg.setSubject("SOME.TEST.SUBJECT");
    float32_t t = 22.0;
    msg.addScalarField("TEST", t);
    msg.addDataField("Name", "Tom");
    msg.addDataField("Name", "Zac");

    fp::MessageBuffer buffer(1024);
    const size_t encoded_len = msg.encode(buffer);
    EXPECT_EQ(buffer.length(), encoded_len);

    DEBUG_LOG(buffer);
}

TEST(Message, Decode) {
    LOG_LEVEL(tf::logger::info);
    fp::MutableMessage in;
    in.setSubject("SOME.TEST.SUBJECT");
    float32_t t = 22.0;
    EXPECT_TRUE(in.addScalarField("TEST1", t));
    EXPECT_TRUE(in.addScalarField("TEST2", true));
    EXPECT_TRUE(in.addDataField("Name1", "Tom"));
    EXPECT_TRUE(in.addDataField("Name2", "Zac"));

    fp::MessageBuffer buffer(1024);
    const size_t encoded_len = in.encode(buffer);
    EXPECT_EQ(buffer.length(), encoded_len);

    DEBUG_LOG(buffer);
    DEBUG_LOG(buffer.byteStorage());
    fp::Message out;
    const fp::MessageBuffer::ByteStorageType &b = buffer.byteStorage();
    EXPECT_TRUE(out.decode(b));
    EXPECT_EQ(encoded_len, b.bytesRead());

    DEBUG_LOG("IN:  " << in);
    DEBUG_LOG("OUT: " << out);
    EXPECT_EQ(in, out);
}

TEST(Message, MultiDecode) {
    LOG_LEVEL(tf::logger::info);

    fp::MutableMessage in1;
    in1.setSubject("SAMPLE.MSG.1");
    float32_t t = 22.0;
    in1.addScalarField("TEST", t);
    in1.addScalarField("TEST", true);
    in1.addDataField("Name", "Tom");
    in1.addDataField("Name", "Zac");

    fp::MutableMessage in2;
    in2.setSubject("SAMPLE.MSG.2");
    t = 26.0;
    in2.addScalarField("TEST", t);
    in2.addScalarField("TEST", false);
    in2.addDataField("Name", "Caroline");
    in2.addDataField("Name", "Heidi");

    fp::MessageBuffer buffer(1024);
    in1.encode(buffer);
    in2.encode(buffer);

    DEBUG_LOG(buffer);

    DEBUG_LOG("Msg 1: " << in1);
    DEBUG_LOG("Msg 2: " << in2);

    fp::Message out;
//    size_t offset = 0;
//    while (buffer.length() != 0 && (offset = out.decode(buffer.byteStorage())) != 0) {
//        std::cout << "Decoded: " << out << std::endl;
//        out.clear();
//        buffer.erase_front(offset);
//    }
}


TEST(Message, MultiPartialDecode) {
    LOG_LEVEL(tf::logger::info);
    fp::MutableMessage in1;
    float32_t t = 22.0;
//    in1.addScalarField("TEST_float", t);
//    in1.addScalarField("TEST_bool", true);
//    in1.addDataField("TEST_string", "Tom is great");
//
    fp::MessageBuffer buffer(1024);
    char subject[256];
    for (int i = 0; i < 10; i++) {
        sprintf(subject, "SAMPLE.MSG.%i", i);
        in1.setSubject(subject);
        in1.addScalarField("TEST_float", t);
        in1.addScalarField("TEST_bool", true);
        in1.addDataField("TEST_string", "Tom is great");
        EXPECT_TRUE(in1.addScalarField("id", i));
        in1.encode(buffer);
        DEBUG_LOG("Encoded buffer is now: " << buffer.length())
        in1.clear();
    }

    DEBUG_LOG(buffer);

    const byte *bytes = nullptr;
    size_t len = 0;
    fp::Message out;
    for (size_t i = 0; i < buffer.length(); i++) {
        len += 10;
        buffer.bytes(&bytes);
        fp::MessageBuffer::ByteStorageType storage(bytes, std::min(len, buffer.length()), true);

//        fp::Message::logMessageBufferDetails(storage);
        bool result = false;
        result = out.decode(storage);
        if (result) {
            buffer.erase_front(storage.bytesRead());
            DEBUG_LOG("Msg decoded: " << out);
            out.clear();
        }
    }
}

TEST(Message, MoveConstructor) {
    LOG_LEVEL(tf::logger::info);

    fp::MutableMessage in1;
    in1.setSubject("SAMPLE.MSG.1");
    float32_t t = 22.0;
    in1.addScalarField("TEST", t);
    in1.addScalarField("TEST", true);
    in1.addDataField("Name", "Tom");
    in1.addDataField("Name", "Zac");

    fp::MessageBuffer buffer1(1024);
    in1.encode(buffer1);

    DEBUG_LOG("Original: " << in1);
    fp::Message in2 = std::move(in1);
    DEBUG_LOG("Moved:    " << in2);

    fp::MessageBuffer buffer2(1024);
    in2.encode(buffer2);

    EXPECT_EQ(buffer1.byteStorage(), buffer2.byteStorage());
}
