#ifndef AUTOCOMPLETE_H
#define AUTOCOMPLETE_H

#include <string>
#include <vector>

class Autocomplete {
public:
    static std::string complete(const std::string& input);
    static std::vector<std::string> suggestions(const std::string& input);

private:
    static const std::vector<std::string> redisCommands;
};

#endif // AUTOCOMPLETE_H
