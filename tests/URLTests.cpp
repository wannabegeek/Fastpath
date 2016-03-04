//
// Created by Tom Fewster on 04/03/2016.
//

#include <gtest/gtest.h>
#include <transport/URL.h>

TEST(URL, ParseCommon) {
    DCF::url url("dcf://localhost:1234");
    EXPECT_STREQ("dcf", url.protocol().c_str());
    EXPECT_STREQ("localhost", url.host().c_str());
    EXPECT_STREQ("", url.path().c_str());
    EXPECT_STREQ("", url.query().c_str());

    DCF::url url2("dcf://localhost:1234/");
    EXPECT_STREQ("dcf", url2.protocol().c_str());
    EXPECT_STREQ("localhost", url2.host().c_str());
    EXPECT_STREQ("/", url2.path().c_str());
    EXPECT_STREQ("", url2.query().c_str());
}

TEST(URL, ParseCommonNoPort) {
    DCF::url url("dcf://localhost");
    EXPECT_STREQ("dcf", url.protocol().c_str());
    EXPECT_STREQ("localhost", url.host().c_str());
    EXPECT_STREQ("", url.path().c_str());
    EXPECT_STREQ("", url.query().c_str());

    DCF::url url2("dcf://localhost/test");
    EXPECT_STREQ("dcf", url2.protocol().c_str());
    EXPECT_STREQ("localhost", url2.host().c_str());
    EXPECT_STREQ("/test", url2.path().c_str());
    EXPECT_STREQ("", url2.query().c_str());
}

TEST(URL, ParseNoProtocol) {
    DCF::url url("localhost:1234");
    EXPECT_STREQ("", url.protocol().c_str());
    EXPECT_STREQ("localhost", url.host().c_str());
    EXPECT_STREQ("1234", url.path().c_str());
    EXPECT_STREQ("", url.query().c_str());
}
