//
// Created by fewstert on 11/03/16.
//

#include <gtest/gtest.h>
#include <router/subscription.h>

TEST(Subscription, ExactMatch) {
    fp::subscription<> subscription("SECTION.ONE.TWO");

    EXPECT_TRUE(subscription.matches(fp::subject<>("SECTION.ONE.TWO")));
    EXPECT_FALSE(subscription.matches(fp::subject<>("SECTION.ONE.THREE")));
}

TEST(Subscription, WildcardElementMatch) {
    {
        fp::subscription<> subscription("SECTION.*.TWO");
        EXPECT_TRUE(subscription.matches("SECTION.ONE.TWO"));
        EXPECT_FALSE(subscription.matches("SECTION.ONE.THREE"));
    }
    {
        fp::subscription<> subscription("SECTION.ONE.*");
        EXPECT_TRUE(subscription.matches("SECTION.ONE.TWO"));
        EXPECT_TRUE(subscription.matches("SECTION.ONE.THREE"));
        EXPECT_FALSE(subscription.matches("SECTION.TWO.THREE"));
        EXPECT_FALSE(subscription.matches("BOOM.ONE.TWO"));
    }
}

TEST(Subscription, WildcardCompletionMatch) {
    {
        fp::subscription<> subscription("SECTION.ONE.>");
        EXPECT_TRUE(subscription.matches("SECTION.ONE.TWO"));
        EXPECT_TRUE(subscription.matches("SECTION.ONE.THREE"));
        EXPECT_FALSE(subscription.matches("SECTION.TWO.THREE"));
    }
    {
        fp::subscription<> subscription("SECTION.>");
        EXPECT_TRUE(subscription.matches("SECTION.ONE.TWO"));
        EXPECT_TRUE(subscription.matches("SECTION.ONE.THREE"));
        EXPECT_TRUE(subscription.matches("SECTION.TWO.THREE"));
        EXPECT_FALSE(subscription.matches("BOOM.ONE.TWO"));
    }
}
