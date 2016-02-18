//
// Created by Tom Fewster on 18/02/2016.
//

#include <gtest/gtest.h>
#include <Message.h>
#include <MessageCodec.h>

TEST(MessageCodec, EncodeSimple) {

    auto msg = std::make_shared<DCF::Message>();
    msg->setSubject("TEST.SUBJECT");
    msg->addField(1234, "TEST");

    DCF::MessageBuffer buffer(1024);
    EXPECT_TRUE(DCF::MessageCodec::encode(msg, buffer));

    std::cout << buffer << std::endl;
}