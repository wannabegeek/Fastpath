//
// Created by Tom Fewster on 15/02/2016.
//

#include <gtest/gtest.h>
#include <Message.h>

TEST(Message, SetSubject) {
    DCF::Message msg;
    const char *subject = "TEST.SUBJECT";
    ASSERT_TRUE(msg.setSubject(subject));

    ASSERT_STREQ(subject, msg.subject());

    ASSERT_TRUE(msg.setSubject("UPDATED.SUBJECT"));
    ASSERT_STRNE(subject, msg.subject());
}

TEST(Message, AddStringField) {

    DCF::Message msg;
    msg.addField(1234, "TEST");

    std::string v;
    ASSERT_TRUE(msg.getField(1234, v));
    ASSERT_STREQ("TEST", v.c_str());

    msg.addField("TEST", "AGAIN");
    ASSERT_TRUE(msg.getField("TEST", v));
    ASSERT_STREQ("AGAIN", v.c_str());
}

TEST(Message, AddFloatField) {
    DCF::Message msg;
    msg.addField("TEST", static_cast<float32_t>(1.4));
    float32_t t = 0.0;
    ASSERT_TRUE(msg.getField("TEST", t));
    ASSERT_FLOAT_EQ(1.4, t);
}

TEST(Message, AddMixedDuplicateField) {
    DCF::Message msg;
    msg.addField("TEST", "AGAIN");

    msg.addField("TEST", static_cast<float32_t>(1.4));
    float32_t t = 0.0;
    std::string v;
    ASSERT_TRUE(msg.getField("TEST", v, 0));
    ASSERT_STREQ("AGAIN", v.c_str());
    ASSERT_TRUE(msg.getField("TEST", t, 1));
    ASSERT_FLOAT_EQ(1.4, t);

}

TEST(Message, RemoveFieldByString) {
    DCF::Message msg;
    float32_t t = 22.0;
    msg.addField("TEST", t);
    ASSERT_TRUE(msg.getField("TEST", t));
    ASSERT_FLOAT_EQ(22, t);

    ASSERT_EQ(1, msg.size());

    ASSERT_TRUE(msg.removeField("TEST"));
    ASSERT_EQ(0, msg.size());
}

TEST(Message, Encode) {
    DCF::Message msg;
    msg.setSubject("SOME.TEST.SUBJECT");
    float32_t t = 22.0;
    msg.addField("TEST", t);
    msg.addField("Name", "Tom");
    msg.addField("Name", "Zac");

    DCF::MessageBuffer buffer(1024);
    msg.encode(buffer);

    std::cout << buffer << std::endl;
}

TEST(Message, Decode) {
    DCF::Message msg;
    msg.setSubject("SOME.TEST.SUBJECT");
    float32_t t = 22.0;
    msg.addField("TEST", t);
    msg.addField("Name", "Tom");
    msg.addField("Name", "Zac");

    DCF::MessageBuffer buffer(1024);
    msg.encode(buffer);

    msg.decode(buffer);
}