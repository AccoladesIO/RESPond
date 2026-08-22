#include <gtest/gtest.h>
#include "CommandHandler.h"

#include <string>
#include <vector>

// ===========================================================================
// CommandHandler::parseCommand
// Tokenises a raw input line. Bare words split on whitespace; a run inside
// double quotes is kept as a single token with the quotes stripped.
// ===========================================================================

TEST(ParseCommand, EmptyInputYieldsNoTokens) {
    EXPECT_TRUE(CommandHandler::parseCommand("").empty());
}

TEST(ParseCommand, WhitespaceOnlyYieldsNoTokens) {
    EXPECT_TRUE(CommandHandler::parseCommand("    \t  ").empty());
}

TEST(ParseCommand, SingleWord) {
    std::vector<std::string> expected{"PING"};
    EXPECT_EQ(CommandHandler::parseCommand("PING"), expected);
}

TEST(ParseCommand, SplitsOnSpaces) {
    std::vector<std::string> expected{"SET", "key", "value"};
    EXPECT_EQ(CommandHandler::parseCommand("SET key value"), expected);
}

TEST(ParseCommand, CollapsesRepeatedSpaces) {
    std::vector<std::string> expected{"SET", "key", "value"};
    EXPECT_EQ(CommandHandler::parseCommand("SET   key    value"), expected);
}

TEST(ParseCommand, IgnoresLeadingAndTrailingSpaces) {
    std::vector<std::string> expected{"GET", "foo"};
    EXPECT_EQ(CommandHandler::parseCommand("   GET foo   "), expected);
}

TEST(ParseCommand, SplitsOnTabs) {
    std::vector<std::string> expected{"SET", "key"};
    EXPECT_EQ(CommandHandler::parseCommand("SET\tkey"), expected);
}

TEST(ParseCommand, KeepsQuotedStringAsOneTokenWithoutQuotes) {
    std::vector<std::string> expected{"SET", "greeting", "hello world"};
    EXPECT_EQ(CommandHandler::parseCommand("SET greeting \"hello world\""), expected);
}

TEST(ParseCommand, QuotedEmptyStringIsAnEmptyToken) {
    std::vector<std::string> expected{"SET", "key", ""};
    EXPECT_EQ(CommandHandler::parseCommand("SET key \"\""), expected);
}

TEST(ParseCommand, QuotedRunOfSpacesIsPreserved) {
    std::vector<std::string> expected{"SET", "key", "   "};
    EXPECT_EQ(CommandHandler::parseCommand("SET key \"   \""), expected);
}

TEST(ParseCommand, MultipleQuotedTokens) {
    std::vector<std::string> expected{"a b", "c d"};
    EXPECT_EQ(CommandHandler::parseCommand("\"a b\" \"c d\""), expected);
}

// ===========================================================================
// CommandHandler::buildRESPCommand
// Serialises an argument vector into a RESP array of bulk strings.
// Layout:  *<n>\r\n  then for each arg:  $<len>\r\n<arg>\r\n
// ===========================================================================

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

// ===========================================================================
// parse -> build round trip (how the two are actually used together in CLI)
// ===========================================================================

TEST(CommandPipeline, ParseThenBuildProducesWireFormat) {
    auto args = CommandHandler::parseCommand("SET greeting \"hello world\"");
    EXPECT_EQ(CommandHandler::buildRESPCommand(args),
              "*3\r\n$3\r\nSET\r\n$8\r\ngreeting\r\n$11\r\nhello world\r\n");
}