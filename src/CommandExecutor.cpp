#include "CommandExecutor.h"
#include "RedisClient.h"
#include "CommandHandler.h"
#include "ResponseParser.h"
#include <iostream>

namespace CommandExecutor {

bool execute(const std::string& host,
             int port,
             const std::vector<std::string>& args,
             int timeout,
             int retries,
             bool verbose) {
    RedisClient redisClient(host, port);
    redisClient.setTimeout(timeout);
    redisClient.setRetries(retries);

    if (!redisClient.connectToServer()) {
        std::cerr << "Failed to connect to Redis server after "
                  << retries << " attempts.\n";
        return false;
    }

    if (verbose) {
        std::cout << "Connected to " << host << ":" << port
                  << " (timeout=" << timeout << "s, retries=" << retries << ")\n";
    }

    std::string command = CommandHandler::buildRESPCommand(args);
    if (!redisClient.sendCommand(command)) {
        std::cerr << "Failed to send command.\n";
        redisClient.disconnectFromServer();
        return false;
    }

    std::string response = ResponseParser::parseResponse(redisClient.getSocketFD());
    std::cout << response << "\n";

    redisClient.disconnectFromServer();
    return true;
}

} // namespace CommandExecutor
