#include <gtest/gtest.h>
#include "Utils.h"

#include <sys/socket.h>
#include <unistd.h>

#include <string>


TEST(Trim, RemovesSurroundingWhitespaceOfAllKinds) {
    EXPECT_EQ(Utils::trim("  \t hi there \r\n "), "hi there");
}

TEST(Trim, LeavesInnerWhitespaceUntouched) {
    EXPECT_EQ(Utils::trim("a  b\tc"), "a  b\tc");
}

TEST(Trim, AllWhitespaceCollapsesToEmpty) {
    EXPECT_EQ(Utils::trim("   \t\r\n "), "");
}

TEST(Trim, EmptyStringStaysEmpty) {
    EXPECT_EQ(Utils::trim(""), "");
}

TEST(Trim, NoWhitespaceIsUnchanged) {
    EXPECT_EQ(Utils::trim("word"), "word");
}


TEST(PrettyJson, EmptyContainersStayInline) {
    EXPECT_EQ(Utils::formatPrettyJson("{}"), "{}");
    EXPECT_EQ(Utils::formatPrettyJson("[]"), "[]");
}

TEST(PrettyJson, IndentsObjectMembers) {
    EXPECT_EQ(Utils::formatPrettyJson("{\"a\": 1}"), "{\n  \"a\": 1\n}");
}

TEST(PrettyJson, IndentsArrayElements) {
    EXPECT_EQ(Utils::formatPrettyJson("[1, 2]"), "[\n  1,\n  2\n]");
}

TEST(PrettyJson, DoesNotFormatBracesInsideStrings) {
    EXPECT_EQ(Utils::formatPrettyJson("{\"a\": \"x{y}z\"}"),
              "{\n  \"a\": \"x{y}z\"\n}");
}

TEST(PrettyJson, PreservesEscapedQuotesInsideStrings) {
    EXPECT_EQ(Utils::formatPrettyJson("{\"a\": \"he \\\"hi\\\"\"}"),
              "{\n  \"a\": \"he \\\"hi\\\"\"\n}");
}


namespace {
int feed(const std::string& data) {
    int fds[2];
    EXPECT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    EXPECT_EQ(::write(fds[1], data.data(), data.size()),
              static_cast<ssize_t>(data.size()));
    ::close(fds[1]);
    return fds[0];
}
}  // namespace

TEST(ReadLine, SplitsOnNewlineAndStripsCarriageReturn) {
    int fd = feed("hello\r\nworld\n");
    EXPECT_EQ(Utils::readLine(fd), "hello");
    EXPECT_EQ(Utils::readLine(fd), "world");
    ::close(fd);
}

TEST(ReadLine, ReturnsEmptyStringForABlankLine) {
    int fd = feed("\r\nnext\r\n");
    EXPECT_EQ(Utils::readLine(fd), "");
    EXPECT_EQ(Utils::readLine(fd), "next");
    ::close(fd);
}

TEST(ReadChar, ReadsOneByteThenSignalsEof) {
    int fd = feed("A");
    char c = 0;
    EXPECT_TRUE(Utils::readChar(fd, c));
    EXPECT_EQ(c, 'A');
    EXPECT_FALSE(Utils::readChar(fd, c));  // peer closed, nothing left
    ::close(fd);
}