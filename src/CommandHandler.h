#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include "RedisClient.h"
#include <string>
#include <vector>

class CommandHandler {
public:
    static std::vector<std::string> parseCommand(const std::string& input);

    // Build a RESP command from the vector arguments

    static std::string buildRESPCommand(const std::vector<std::string> &args);
};

#endif // COMMAND_HANDLER_H
