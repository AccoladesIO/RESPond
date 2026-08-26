#include <gtest/gtest.h>
#include "CommandHandler.h"

#include <string>
#include <vector>


TEST(ParseCommand, EmptyInputYieldsNoTokens) {
    EXPECT_TRUE(CommandHandler::parseCommand("").empty());
}

TEST(ParseCommand, WhitespaceOnlyYieldsNoTokens) {
    EXPECT_TRUE(CommandHandler::parseCommand("    \t  ").empty());
}

TEST(ParseCommand, SingleWord) {
    EXPECT_EQ(CommandHandler::parseCommand("PING"),
              (std::vector<std::string>{"PING"}));
}

TEST(ParseCommand, SplitsOnArbitraryWhitespace) {
    EXPECT_EQ(CommandHandler::parseCommand("  SET\t  key    value  "),
              (std::vector<std::string>{"SET", "key", "value"}));
}

TEST(ParseCommand, KeepsQuotedStringAsOneTokenWithoutQuotes) {
    EXPECT_EQ(CommandHandler::parseCommand("SET greeting \"hello world\""),
              (std::vector<std::string>{"SET", "greeting", "hello world"}));
}

TEST(ParseCommand, QuotedEmptyStringIsAnEmptyToken) {
    EXPECT_EQ(CommandHandler::parseCommand("SET key \"\""),
              (std::vector<std::string>{"SET", "key", ""}));
}

TEST(ParseCommand, QuotedRunOfSpacesIsPreserved) {
    EXPECT_EQ(CommandHandler::parseCommand("SET key \"   \""),
              (std::vector<std::string>{"SET", "key", "   "}));
}

TEST(ParseCommand, MixesQuotedAndBareTokens) {
    EXPECT_EQ(CommandHandler::parseCommand("HSET \"my hash\" field val"),
              (std::vector<std::string>{"HSET", "my hash", "field", "val"}));
}

TEST(ParseCommand, PunctuationInsideAWordStaysAttached) {
    EXPECT_EQ(CommandHandler::parseCommand("GET user:1000:name"),
              (std::vector<std::string>{"GET", "user:1000:name"}));
}

TEST(BuildRESPCommand, EmptyArgsIsEmptyArray) {
    EXPECT_EQ(CommandHandler::buildRESPCommand({}), "*0\r\n");
}

TEST(BuildRESPCommand, SingleArgument) {
    EXPECT_EQ(CommandHandler::buildRESPCommand({"PING"}),
              "*1\r\n$4\r\nPING\r\n");
}

TEST(BuildRESPCommand, ThreeArguments) {
    EXPECT_EQ(CommandHandler::buildRESPCommand({"SET", "key", "value"}),
              "*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$5\r\nvalue\r\n");
}

TEST(BuildRESPCommand, LengthCountsSpacesInsideArgument) {
    EXPECT_EQ(CommandHandler::buildRESPCommand({"SET", "key", "hello world"}),
              "*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$11\r\nhello world\r\n");
}

TEST(BuildRESPCommand, EmptyArgumentIsZeroLengthBulkString) {
    EXPECT_EQ(CommandHandler::buildRESPCommand({"SET", "key", ""}),
              "*3\r\n$3\r\nSET\r\n$3\r\nkey\r\n$0\r\n\r\n");
}

TEST(BuildRESPCommand, LengthPrefixIsByteCountNotCharCount) {
    EXPECT_EQ(CommandHandler::buildRESPCommand({"ECHO", "123456789012"}),
              "*2\r\n$4\r\nECHO\r\n$12\r\n123456789012\r\n");
}

TEST(BuildRESPCommand, IsBinarySafeForEmbeddedCrlf) {
    EXPECT_EQ(CommandHandler::buildRESPCommand({"SET", "k", "a\r\nb"}),
              "*3\r\n$3\r\nSET\r\n$1\r\nk\r\n$4\r\na\r\nb\r\n");
}


TEST(CommandPipeline, ParseThenBuildProducesWireFormat) {
    auto args = CommandHandler::parseCommand("SET greeting \"hello world\"");
    EXPECT_EQ(CommandHandler::buildRESPCommand(args),
              "*3\r\n$3\r\nSET\r\n$8\r\ngreeting\r\n$11\r\nhello world\r\n");
}