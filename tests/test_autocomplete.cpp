#include <gtest/gtest.h>
#include "AutoComplete.h"

#include <string>
#include <vector>

TEST(Suggestions, UniquePrefixReturnsExactlyOne) {
    EXPECT_EQ(Autocomplete::suggestions("PI"),
              (std::vector<std::string>{"PING"}));
}

TEST(Suggestions, SharedPrefixReturnsAllMatchesInTableOrder) {
    EXPECT_EQ(Autocomplete::suggestions("D"),
              (std::vector<std::string>{"DEL", "DECR", "DBSIZE", "DISCARD"}));
}

TEST(Suggestions, NoMatchReturnsEmpty) {
    EXPECT_TRUE(Autocomplete::suggestions("XYZ").empty());
}

TEST(Suggestions, MatchingIsCaseSensitive) {
    // Lower-case does not match the upper-case table — documents current behaviour.
    EXPECT_TRUE(Autocomplete::suggestions("ping").empty());
}

TEST(Suggestions, EmptyInputMatchesEverything) {
    // Every command "starts with" the empty string.
    EXPECT_EQ(Autocomplete::suggestions("").size(), 20u);
}

TEST(Complete, UniquePrefixExpandsToFullCommand) {
    EXPECT_EQ(Autocomplete::complete("SUB"), "SUBSCRIBE");
    EXPECT_EQ(Autocomplete::complete("DB"), "DBSIZE");
}

TEST(Complete, AmbiguousPrefixIsReturnedUnchanged) {
    // "DE" -> {DEL, DECR}: more than one match, so no expansion.
    EXPECT_EQ(Autocomplete::complete("DE"), "DE");
}

TEST(Complete, NoMatchIsReturnedUnchanged) {
    EXPECT_EQ(Autocomplete::complete("XYZ"), "XYZ");
}

TEST(Complete, EmptyInputIsReturnedUnchanged) {
    EXPECT_EQ(Autocomplete::complete(""), "");
}

TEST(Complete, FullCommandThatIsAlsoAPrefixIsNotOverExpanded) {
    EXPECT_EQ(Autocomplete::complete("DEL"), "DEL");
}