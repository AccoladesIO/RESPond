#include "AutoComplete.h"
#include <algorithm>

const std::vector<std::string> Autocomplete::redisCommands = {
    "PING", "SET", "GET", "DEL", "ECHO", "INCR", "DECR",
    "LPUSH", "RPUSH", "LRANGE", "MGET", "EXISTS", "DBSIZE",
    "FLUSHDB", "FLUSHALL", "MULTI", "EXEC", "DISCARD", "SUBSCRIBE", "PUBLISH"};

std::string Autocomplete::complete(const std::string &input)
{
    if (input.empty())
        return input;

    std::vector<std::string> matches = suggestions(input);
    if (matches.size() == 1)
    {
        return matches[0]; // unique match
    }
    return input; // no unique match, return as-is
}

std::vector<std::string> Autocomplete::suggestions(const std::string &input)
{
    std::vector<std::string> results;
    for (const auto &cmd : redisCommands)
    {
        if (cmd.find(input) == 0)
        { // starts with input
            results.push_back(cmd);
        }
    }
    return results;
}
