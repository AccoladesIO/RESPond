#include <gtest/gtest.h>
#include "ResponseParser.h"

#include <sys/socket.h>
#include <unistd.h>

#include <string>

namespace {

std::string parse(const std::string& raw, bool pretty = false) {
    int fds[2];
    EXPECT_EQ(::socketpair(AF_UNIX, SOCK_STREAM, 0, fds), 0);
    ssize_t w = ::write(fds[1], raw.data(), raw.size());
    EXPECT_EQ(w, static_cast<ssize_t>(raw.size()));
    ::close(fds[1]);  // EOF, so any over-read is detected rather than blocking
    std::string out = ResponseParser::parseResponse(fds[0], pretty);
    ::close(fds[0]);
    return out;
}

} 

TEST(ParseResponse, SimpleString) {
    EXPECT_EQ(parse("+OK\r\n"), "\"OK\"");
}

TEST(ParseResponse, SimpleError) {
    EXPECT_EQ(parse("-ERR bad arg\r\n"), "{\"error\": \"ERR bad arg\"}");
}

TEST(ParseResponse, Integer) {
    EXPECT_EQ(parse(":1000\r\n"), "1000");
}

TEST(ParseResponse, NegativeInteger) {
    EXPECT_EQ(parse(":-42\r\n"), "-42");
}

TEST(ParseResponse, Double) {
    EXPECT_EQ(parse(",3.14\r\n"), "3.14");
}

TEST(ParseResponse, BigNumberIsQuoted) {
    EXPECT_EQ(parse("(12345678901234567890\r\n"), "\"12345678901234567890\"");
}

TEST(ParseResponse, BooleanTrue) {
    EXPECT_EQ(parse("#t\r\n"), "true");
}

TEST(ParseResponse, BooleanFalse) {
    EXPECT_EQ(parse("#f\r\n"), "false");
}

TEST(ParseResponse, Null) {
    EXPECT_EQ(parse("_\r\n"), "null");
}

// --- Bulk / verbatim strings ----------------------------------------------

TEST(ParseResponse, BulkString) {
    EXPECT_EQ(parse("$5\r\nhello\r\n"), "\"hello\"");
}

TEST(ParseResponse, EmptyBulkString) {
    EXPECT_EQ(parse("$0\r\n\r\n"), "\"\"");
}

TEST(ParseResponse, NullBulkString) {
    EXPECT_EQ(parse("$-1\r\n"), "null");
}

TEST(ParseResponse, VerbatimStringStripsThreeByteTypePrefix) {
    // =<len>\r\n<3-char type>:<payload>\r\n  ->  payload only
    EXPECT_EQ(parse("=15\r\ntxt:Some string\r\n"), "\"Some string\"");
}

// --- Aggregates ------------------------------------------------------------

TEST(ParseResponse, ArrayOfIntegers) {
    EXPECT_EQ(parse("*2\r\n:1\r\n:2\r\n"), "[1, 2]");
}

TEST(ParseResponse, ArrayOfBulkStrings) {
    EXPECT_EQ(parse("*2\r\n$3\r\nfoo\r\n$3\r\nbar\r\n"), "[\"foo\", \"bar\"]");
}

TEST(ParseResponse, EmptyArray) {
    EXPECT_EQ(parse("*0\r\n"), "[]");
}

TEST(ParseResponse, NullArray) {
    EXPECT_EQ(parse("*-1\r\n"), "null");
}

TEST(ParseResponse, NestedArray) {
    EXPECT_EQ(parse("*2\r\n*2\r\n:1\r\n:2\r\n$3\r\nfoo\r\n"), "[[1, 2], \"foo\"]");
}

TEST(ParseResponse, SetRendersLikeArray) {
    EXPECT_EQ(parse("~2\r\n:1\r\n:2\r\n"), "[1, 2]");
}

TEST(ParseResponse, MapRendersAsObject) {
    EXPECT_EQ(parse("%1\r\n$3\r\nfoo\r\n$3\r\nbar\r\n"), "{\"foo\": \"bar\"}");
}

TEST(ParseResponse, PushMessage) {
    EXPECT_EQ(parse(">2\r\n$7\r\nmessage\r\n$5\r\nhello\r\n"),
              "{\"push\": [\"message\", \"hello\"]}");
}

// --- Escaping --------------------------------------------------------------

TEST(ParseResponse, EscapesEmbeddedDoubleQuote) {
    // Bulk payload a"b must be emitted as JSON "a\"b".
    EXPECT_EQ(parse("$3\r\na\"b\r\n"), "\"a\\\"b\"");
}

// --- Error / edge conditions ----------------------------------------------

TEST(ParseResponse, UnknownTypeByteReportsError) {
    EXPECT_EQ(parse("!oops\r\n"),
              "{\"error\": \"Unknown response type: !\"}");
}

TEST(ParseResponse, ClosedConnectionReportsError) {
    EXPECT_EQ(parse(""), "{\"error\": \"No response or connection closed\"}");
}

TEST(ParseResponse, BulkStringWithColonIsTreatedAsInfoKeyValue) {
    EXPECT_EQ(parse("$7\r\nk:v arg\r\n"), "{\"k\": \"v arg\"}");
}

TEST(ParseResponse, InfoBlockBecomesNestedObject) {
    const std::string info = "# Server\r\nredis_version:7.0.0\r\nos:Linux\r\n";
    const std::string frame = "$" + std::to_string(info.size()) + "\r\n" + info + "\r\n";
    EXPECT_EQ(parse(frame),
              "{\"Server\": {\"redis_version\": \"7.0.0\", \"os\": \"Linux\"}}");
}

// --- Pretty printing integration ------------------------------------------

TEST(ParseResponse, PrettyPrintsAnArray) {
    EXPECT_EQ(parse("*2\r\n:1\r\n:2\r\n", /*pretty=*/true),
              "[\n  1,\n  2\n]");
}