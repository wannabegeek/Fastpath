//
// Created by Tom Fewster on 15/02/2016.
//

#include <gtest/gtest.h>
#include <Message.h>

TEST(Elements, CreateString) {

    DCF::Field e;
    e.setValue(std::string("test"));
    ASSERT_EQ(e.type(), DCF::StorageType::string);
    std::string result;
    ASSERT_TRUE(e.get(result));
//    ASSERT_STREQ((const char *)result, temp);

    // set the string as a char * & retrive it as a std::string
    const char *b = "fgsg";
    DCF::Field e3;
    e3.setValue(b);
    ASSERT_EQ(e3.type(), DCF::StorageType::string);
    std::string result3;
    ASSERT_TRUE(e3.get(result3));

    // set the string as a char * & retrive it as a char *
    const char *b4 = "test";
    DCF::Field e4;
    e4.setValue(b4);
    ASSERT_EQ(e4.type(), DCF::StorageType::string);
    const char *result4;
    ASSERT_TRUE(e4.get(&result4));
    ASSERT_STREQ(b4, result4);

}

TEST(Elements, CreateInt32) {
    DCF::Field e;
    int32_t t = 42;
    e.setValue(t);
    ASSERT_EQ(e.type(), DCF::StorageType::int32);
    int32_t result;
    ASSERT_TRUE(e.get(result));
    ASSERT_EQ(t, result);
}

TEST(Elements, CreateInt64) {
    DCF::Field e;
    int64_t t = 42;
    e.setValue(t);
    ASSERT_EQ(e.type(), DCF::StorageType::int64);
    int64_t result;
    ASSERT_TRUE(e.get(result));
    ASSERT_EQ(t, result);
}

TEST(Elements, CreateFloat32) {
    DCF::Field e;
    float32_t t = 42.999;
    e.setValue(t);
    ASSERT_EQ(e.type(), DCF::StorageType::float32);
    float32_t result;
    ASSERT_TRUE(e.get(result));
    ASSERT_FLOAT_EQ(t, result);
}

TEST(Elements, CreateFloat64) {
    DCF::Field e;
    float64_t t = 42.999;
    e.setValue(t);
    ASSERT_EQ(e.type(), DCF::StorageType::float64);
    float64_t result;
    ASSERT_TRUE(e.get(result));
    ASSERT_FLOAT_EQ(t, result);
}


TEST(Elements, CreateData) {
    const char *temp = "Hello world";

    DCF::Field e;
    e.setValue(reinterpret_cast<const byte *>(temp), strlen(temp));
    ASSERT_EQ(e.type(), DCF::StorageType::data);

    const byte *result = nullptr;
    size_t len = 0;
    ASSERT_TRUE(e.get(&result, len));
    ASSERT_EQ(strlen(temp), len);
    ASSERT_NE(reinterpret_cast<const char *>(result), temp);  // Pointers can't be the same, data should have been copied
    ASSERT_EQ(0, strncmp(reinterpret_cast<const char *>(result), temp, strlen(temp)));

    std::string result2;
    ASSERT_FALSE(e.get(result2));  // this should fail since we are getting something with a different type
}