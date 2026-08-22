#include "CommandHandler.h"
#include <sstream>
#include <regex>

std::vector<std::string> CommandHandler::parseCommand(const std::string& input) {
    std::vector<std::string> tokens;

    // Regex: match quoted strings ("...") or non-space words
    std::regex rgx(R"delim("([^"]*)"|(\S+))delim");

    auto words_begin = std::sregex_iterator(input.begin(), input.end(), rgx);
    auto words_end = std::sregex_iterator();

    for (auto it = words_begin; it != words_end; ++it) {
        if ((*it)[1].matched) {
            // Quoted string without quotes
            tokens.push_back((*it)[1].str());
        } else {
            // Regular word
            tokens.push_back((*it)[2].str());
        }
    }

    return tokens;
};

/*
* start of an array
* bulk of string
args
*/

std::string CommandHandler::buildRESPCommand(const std::vector<std::string> &args){
    std::ostringstream oss;
    oss << "*" << args.size() << "\r\n";

    for(const auto &arg: args){
        oss << "$" << arg.size() << "\r\n" << arg << "\r\n";
    }
    return oss.str();
}

