//
// Created by Tom Fewster on 15/02/2016.
//

#include <gtest/gtest.h>
#include <messages/Message.h>
#include <messages/DateTimeField.h>
#include <messages/MessageField.h>

TEST(Field, CreateString_Large) {

    // set the string as a char * &
    const char *b = "fgsg";
    std::allocator<byte> alloc;
    DCF::LargeDataField<decltype(alloc)> e3("0", b, alloc);
    ASSERT_EQ(e3.type(), DCF::StorageType::string);
    ASSERT_STREQ(b, e3.get<const char *>());

    // set the string as a char * & retrive it as a char *
    const char *b4 = "test";
    DCF::LargeDataField<decltype(alloc)> e4("0", b4, alloc);
    ASSERT_EQ(e4.type(), DCF::StorageType::string);
    const char *result4 = nullptr;
    ASSERT_EQ(5u, e4.get(&result4));
    ASSERT_STREQ(b4, result4);
}

TEST(Field, CreateString_Small) {

    // set the string as a char * &
    const char *b = "fgsg";
    DCF::SmallDataField e3("0", b);
    ASSERT_EQ(e3.type(), DCF::StorageType::string);
    ASSERT_STREQ(b, e3.get<const char *>());

    // set the string as a char * & retrive it as a char *
    const char *b4 = "test";
    DCF::SmallDataField e4("0", b4);
    ASSERT_EQ(e4.type(), DCF::StorageType::string);
    const char *result4 = nullptr;
    ASSERT_EQ(5u, e4.get(&result4));
    ASSERT_STREQ(b4, result4);
}

TEST(Field, CreateInt32) {
    int32_t t = 42;
    DCF::ScalarField e("0", t);
    ASSERT_EQ(e.type(), DCF::StorageType::int32);
    ASSERT_EQ(t, e.get<int32_t>());
}

TEST(Field, CreateInt64) {
    int64_t t = 42;
    DCF::ScalarField e("0", t);
    ASSERT_EQ(e.type(), DCF::StorageType::int64);
    ASSERT_EQ(t, e.get<int64_t>());
}

TEST(Field, CreateFloat32) {
    float32_t t = 42.999;
    DCF::ScalarField e("0", t);
    ASSERT_EQ(e.type(), DCF::StorageType::float32);
    ASSERT_FLOAT_EQ(t, e.get<float32_t>());
}

TEST(Field, CreateFloat64) {
    float64_t t = 42.999;
    DCF::ScalarField e("0", t);
    ASSERT_EQ(e.type(), DCF::StorageType::float64);
    ASSERT_FLOAT_EQ(t, e.get<float64_t>());
}

TEST(Field, CreateDateTime) {
    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
    DCF::DateTimeField e("time", time);
    ASSERT_EQ(e.type(), DCF::StorageType::date_time);
    ASSERT_EQ(time, e.get());
}


TEST(Field, CreateData_Large) {
    const char *temp = "Hello world";

    std::allocator<byte> alloc;
    DCF::LargeDataField<decltype(alloc)> e("0", reinterpret_cast<const byte *>(temp), strlen(temp), alloc);
    ASSERT_EQ(e.type(), DCF::StorageType::data);

    const byte *result = nullptr;
    ASSERT_EQ(strlen(temp), e.get(&result));
    ASSERT_NE(reinterpret_cast<const char *>(result), temp);  // Pointers can't be the same, data should have been copied
    ASSERT_EQ(0, strncmp(reinterpret_cast<const char *>(result), temp, strlen(temp)));
}

TEST(Field, CreateData_Small) {
    const char *temp = "Hello world";

    DCF::SmallDataField e("0", reinterpret_cast<const byte *>(temp), strlen(temp));
    ASSERT_EQ(e.type(), DCF::StorageType::data);

    const byte *result = nullptr;
    ASSERT_EQ(strlen(temp), e.get(&result));
    ASSERT_NE(reinterpret_cast<const char *>(result), temp);  // Pointers can't be the same, data should have been copied
    ASSERT_EQ(0, strncmp(reinterpret_cast<const char *>(result), temp, strlen(temp)));
}

TEST(Field, SerializeString_Large) {
    const char *temp = "Hello world";
    DCF::LargeDataField<std::allocator<byte>> in("0", reinterpret_cast<const byte *>(temp), strlen(temp));

    DCF::MessageBuffer buffer(256);
    const size_t len_in = in.encode(buffer);
    EXPECT_EQ(len_in, buffer.length());

    const DCF::MessageBuffer::ByteStorageType &b = buffer.byteStorage();
    DCF::LargeDataField<std::allocator<byte>> out(b);
//    EXPECT_TRUE(out.decode(b));
    ASSERT_EQ(len_in, b.bytesRead());

    EXPECT_EQ(in, out);
}

TEST(Field, SerializeString_Small) {
    const char *temp = "Hello world";
    DCF::SmallDataField in("0", reinterpret_cast<const byte *>(temp), strlen(temp));

    DCF::MessageBuffer buffer(256);
    const size_t len_in = in.encode(buffer);
    EXPECT_EQ(len_in, buffer.length());

    const DCF::MessageBuffer::ByteStorageType &b = buffer.byteStorage();
    DCF::SmallDataField out(b);
//    EXPECT_TRUE(out.decode(b));
    ASSERT_EQ(len_in, b.bytesRead());

    EXPECT_EQ(in, out);
}
TEST(Field, SerializeScalar) {
    const uint32_t temp = 1234567890u;
    DCF::ScalarField in("0", temp);

    DCF::MessageBuffer buffer(256);
    const size_t len_in = in.encode(buffer);
    EXPECT_EQ(len_in, buffer.length());

    const DCF::MessageBuffer::ByteStorageType &b = buffer.byteStorage();
    DCF::ScalarField out(b);
//    EXPECT_TRUE(out.decode(b));
    EXPECT_EQ(len_in, b.bytesRead());

    EXPECT_EQ(in, out);
}

TEST(Field, SerializeDateTime) {
    std::chrono::time_point<std::chrono::system_clock> time = std::chrono::system_clock::now();
    DCF::DateTimeField in("0", time);

    DCF::MessageBuffer buffer(256);
    const size_t len_in = in.encode(buffer);
    EXPECT_EQ(len_in, buffer.length());

    const DCF::MessageBuffer::ByteStorageType &b = buffer.byteStorage();
    DCF::DateTimeField out(b);
//    EXPECT_TRUE(out.decode(b));
    EXPECT_EQ(len_in, b.bytesRead());

    EXPECT_EQ(in, out);
}

TEST(Field, SerializeSubMessage) {

    LOG_LEVEL(tf::logger::info);

    DCF::Message msg;
    float32_t t = 22.0;
    EXPECT_TRUE(msg.addScalarField("TEST1", t));
    EXPECT_TRUE(msg.addScalarField("TEST2", true));
    EXPECT_TRUE(msg.addDataField("Name1", "Tom"));
    EXPECT_TRUE(msg.addDataField("Name2", "Zac"));

    DEBUG_LOG("Passing in " << msg);

    DCF::MessageField in("msg", std::move(msg));

    DCF::MessageBuffer buffer(256);
    const size_t len_in = in.encode(buffer);
    EXPECT_EQ(len_in, buffer.length());

    DEBUG_LOG("Buffer is: " << buffer);
    DCF::MessageField out(buffer.byteStorage());

//    EXPECT_EQ(len_in, b.bytesRead());
//
    DEBUG_LOG("IN:  " << in);
    DEBUG_LOG("OUT: " << out);
    EXPECT_EQ(in, out);
}