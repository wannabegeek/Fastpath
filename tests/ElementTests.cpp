//
// Created by Tom Fewster on 15/02/2016.
//

#include <gtest/gtest.h>
#include <Message.h>

TEST(Elements, Create) {

    const char *temp = "Hello world";

    DCF::Element e;
    e.setValue(static_cast<const void *>(temp), strlen(temp));
    ASSERT_EQ(e.type(), DCF::StorageType::data);

    const void *result = nullptr;
    ASSERT_TRUE(e.get(result));
    ASSERT_STREQ((const char *)result, temp);

    std::string result2;
    ASSERT_FALSE(e.get(result2)); // this should fail since we are getting something with a different type

    DCF::Element e2;
    e2.setValue(std::string("test"));
    ASSERT_EQ(e2.type(), DCF::StorageType::string);
    ASSERT_TRUE(e2.get(result2));
//    ASSERT_STREQ((const char *)result, temp);

    const char *b = "fgsg";
    DCF::Element e3;
    e3.setValue(b);
    ASSERT_EQ(e3.type(), DCF::StorageType::string);
    std::string result3;
    ASSERT_TRUE(e3.get(result3));


}