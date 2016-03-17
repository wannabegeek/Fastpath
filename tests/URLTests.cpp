//
// Created by Tom Fewster on 04/03/2016.
//

#include <gtest/gtest.h>
#include <transport/URL.h>

TEST(URL, ParseCommon) {
    DCF::url url("dcf://localhost:1234");
    EXPECT_STREQ("dcf", url.protocol().c_str());
    EXPECT_STREQ("localhost", url.host().c_str());
    EXPECT_STREQ("1234", url.port().c_str());
    EXPECT_STREQ("", url.path().c_str());
    EXPECT_EQ(0u, url.query().size());

    DCF::url url2("dcf://localhost:1234/");
    EXPECT_STREQ("dcf", url2.protocol().c_str());
    EXPECT_STREQ("localhost", url2.host().c_str());
    EXPECT_STREQ("1234", url.port().c_str());
    EXPECT_STREQ("", url2.path().c_str());
    EXPECT_EQ(0u, url2.query().size());
}

TEST(URL, ParseCommonNoPort) {
    DCF::url url("dcf://localhost");
    EXPECT_STREQ("dcf", url.protocol().c_str());
    EXPECT_STREQ("localhost", url.host().c_str());
    EXPECT_STREQ("", url.port().c_str());
    EXPECT_STREQ("", url.path().c_str());
    EXPECT_EQ(0u, url.query().size());

    DCF::url url2("dcf://localhost/test");
    EXPECT_STREQ("dcf", url2.protocol().c_str());
    EXPECT_STREQ("localhost", url2.host().c_str());
    EXPECT_STREQ("", url.port().c_str());
    EXPECT_STREQ("test", url2.path().c_str());
    EXPECT_EQ(0u, url2.query().size());
}

TEST(URL, ParseNoProtocol) {
    EXPECT_THROW(DCF::url url("localhost:1234"), std::invalid_argument);
    EXPECT_THROW(DCF::url url2("localhost:1234/test"), std::invalid_argument);
}

TEST(URL, ParseCommonQuery) {
    {
        DCF::url url("dcf://localhost?a");
        EXPECT_STREQ("dcf", url.protocol().c_str());
        EXPECT_STREQ("localhost", url.host().c_str());
        EXPECT_STREQ("", url.port().c_str());
        EXPECT_STREQ("", url.path().c_str());
        EXPECT_EQ(1u, url.query().size());
        EXPECT_STREQ("", url.query().at("a").c_str());
    }

    {
        DCF::url url("dcf://localhost:1234?a");
        EXPECT_STREQ("dcf", url.protocol().c_str());
        EXPECT_STREQ("localhost", url.host().c_str());
        EXPECT_STREQ("1234", url.port().c_str());
        EXPECT_STREQ("", url.path().c_str());
        EXPECT_EQ(1u, url.query().size());
        EXPECT_STREQ("", url.query().at("a").c_str());
    }

    {
        DCF::url url("dcf://localhost/test?a=1");
        EXPECT_STREQ("dcf", url.protocol().c_str());
        EXPECT_STREQ("localhost", url.host().c_str());
        EXPECT_STREQ("", url.port().c_str());
        EXPECT_STREQ("test", url.path().c_str());
        EXPECT_EQ(1u, url.query().size());
        EXPECT_STREQ("1", url.query().at("a").c_str());
    }

    {
        DCF::url url("dcf://localhost:1234/test?a=1");
        EXPECT_STREQ("dcf", url.protocol().c_str());
        EXPECT_STREQ("localhost", url.host().c_str());
        EXPECT_STREQ("1234", url.port().c_str());
        EXPECT_STREQ("test", url.path().c_str());
        EXPECT_EQ(1u, url.query().size());
        EXPECT_STREQ("1", url.query().at("a").c_str());
    }

    {
        DCF::url url("dcf://localhost/test?a=1&b=2");
        EXPECT_STREQ("dcf", url.protocol().c_str());
        EXPECT_STREQ("localhost", url.host().c_str());
        EXPECT_STREQ("", url.port().c_str());
        EXPECT_STREQ("test", url.path().c_str());
        EXPECT_EQ(2u, url.query().size());
        EXPECT_STREQ("1", url.query().at("a").c_str());
        EXPECT_STREQ("2", url.query().at("b").c_str());
    }

    {
        DCF::url url("dcf://localhost:1234/test?a=1&b=2");
        EXPECT_STREQ("dcf", url.protocol().c_str());
        EXPECT_STREQ("localhost", url.host().c_str());
        EXPECT_STREQ("1234", url.port().c_str());
        EXPECT_STREQ("test", url.path().c_str());
        EXPECT_EQ(2u, url.query().size());
        EXPECT_STREQ("1", url.query().at("a").c_str());
        EXPECT_STREQ("2", url.query().at("b").c_str());
    }

}
