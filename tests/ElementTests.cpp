//
// Created by Tom Fewster on 15/02/2016.
//

#include <gtest/gtest.h>
#include <Message.h>

TEST(Elements, CreateString) {

    DCF::Element e;
    e.setValue(std::string("test"));
    ASSERT_EQ(e.type(), DCF::StorageType::string);
    std::string result;
    ASSERT_TRUE(e.get(result));
//    ASSERT_STREQ((const char *)result, temp);

    // set the string as a char * & retrive it as a std::string
    const char *b = "fgsg";
    DCF::Element e3;
    e3.setValue(b);
    ASSERT_EQ(e3.type(), DCF::StorageType::string);
    std::string result3;
    ASSERT_TRUE(e3.get(result3));

    // set the string as a char * & retrive it as a char *
    const char *b4 = "test";
    DCF::Element e4;
    e4.setValue(b4);
    ASSERT_EQ(e4.type(), DCF::StorageType::string);
    const char *result4;
    ASSERT_TRUE(e4.get(&result4));
    ASSERT_STREQ(b4, result4);

}

TEST(Elements, CreateInt) {
    DCF::Element e;
    e.setValue(42);
    ASSERT_EQ(e.type(), DCF::StorageType::int32);
    int32_t result;
    ASSERT_TRUE(e.get(result));
    ASSERT_EQ(result, 42);
}

TEST(Elements, CreateData) {
    const char *temp = "Hello world";

    DCF::Element e;
    e.setValue(static_cast<const void *>(temp), strlen(temp));
    ASSERT_EQ(e.type(), DCF::StorageType::data);

    const void *result = nullptr;
    ASSERT_TRUE(e.get(result));
    ASSERT_NE(result, temp);  // Pointers can't be the same, data should have been copied
    ASSERT_STREQ(static_cast<const char *>(result), temp);

    std::string result2;
    ASSERT_FALSE(e.get(result2));  // this should fail since we are getting something with a different type
}