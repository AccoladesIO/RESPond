#ifndef CLI_H
#define CLI_H
#include "RedisClient.h"
#include <vector> 
#include <string>
#include "CommandHandler.h"
#include "ResponseParser.h"

class CLI {
public:
    CLI(const std::string& host, int port);
    void run();

private:
    RedisClient redisClient;
    std::vector<std::string> history;
};

#endif // CLI_H