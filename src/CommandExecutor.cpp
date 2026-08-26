#include "CommandExecutor.h"
#include "RedisClient.h"
#include "CommandHandler.h"
#include "ResponseParser.h"
#include <iostream>
#include <sstream>
#include <fstream>
#include "Utils.h"

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

bool executePipeline(const std::string& host,
                     int port,
                     const std::vector<std::vector<std::string>>& commands,
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

    // Build batch of commands
    std::ostringstream batch;
    for (const auto& args : commands) {
        batch << CommandHandler::buildRESPCommand(args);
    }

    if (!redisClient.sendCommand(batch.str())) {
        std::cerr << "Failed to send pipeline batch.\n";
        redisClient.disconnectFromServer();
        return false;
    }

    // Parse replies in sequence
    for (size_t i = 0; i < commands.size(); ++i) {
        std::string response = ResponseParser::parseResponse(redisClient.getSocketFD());
        std::cout << i << ") " << response << "\n";
    }

    redisClient.disconnectFromServer();
    return true;
}

bool executeScript(const std::string& host,
                   int port,
                   const std::string& scriptPath,
                   int timeout,
                   int retries,
                   bool verbose) {
    std::ifstream file(scriptPath);
    if (!file.is_open()) {
        std::cerr << "Failed to open script file: " << scriptPath << "\n";
        return false;
    }

    std::vector<std::vector<std::string>> commands;
    std::string line;
    while (std::getline(file, line)) {
        line = Utils::trim(line);
        if (line.empty() || line[0] == '#') continue; // skip blanks/comments
        std::vector<std::string> args = CommandHandler::parseCommand(line);
        if (!args.empty()) commands.push_back(args);
    }

    file.close();

    if (commands.empty()) {
        std::cerr << "No commands found in script.\n";
        return false;
    }

    return executePipeline(host, port, commands, timeout, retries, verbose);
}

bool executeInlineScript(const std::string& host,
                         int port,
                         const std::string& scriptBody,
                         int timeout,
                         int retries,
                         bool verbose) {
    std::stringstream ss(scriptBody);
    std::string segment;
    std::vector<std::vector<std::string>> commands;

    while (std::getline(ss, segment, ';')) {
        segment = Utils::trim(segment);
        if (segment.empty()) continue;
        std::vector<std::string> args = CommandHandler::parseCommand(segment);
        if (!args.empty()) commands.push_back(args);
    }

    if (commands.empty()) {
        std::cerr << "No commands found in inline script.\n";
        return false;
    }

    return executePipeline(host, port, commands, timeout, retries, verbose);
}
} // namespace CommandExecutor
