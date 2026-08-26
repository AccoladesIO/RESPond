#include <gtest/gtest.h>
#include "Config.h"

TEST(ConfigTest, SaveAndLoadConfiguration) {
    std::string testHost = "127.0.0.1";
    int testPort = 6379;

    Config::save(testHost, testPort);

    std::string loadedHost;
    int loadedPort = 0;
    bool success = Config::load(loadedHost, loadedPort);

    EXPECT_TRUE(success);
    EXPECT_EQ(loadedHost, testHost);
    EXPECT_EQ(loadedPort, testPort);
}

TEST(ConfigTest, SaveAndLoadCustomValues) {
    std::string testHost = "192.168.1.50";
    int testPort = 6380;

    Config::save(testHost, testPort);

    std::string loadedHost;
    int loadedPort = 0;
    bool success = Config::load(loadedHost, loadedPort);

    EXPECT_TRUE(success);
    EXPECT_EQ(loadedHost, testHost);
    EXPECT_EQ(loadedPort, testPort);
}