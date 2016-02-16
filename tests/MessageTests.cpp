//
// Created by Tom Fewster on 15/02/2016.
//

#include <gtest/gtest.h>
#include <Message.h>

TEST(Message, AddField) {

    DCF::Message msg;
    msg.addField(1234, "TEST");

//    EXPECT_GT(110, actual.count());
//    EXPECT_LT(99, actual.count());
}
