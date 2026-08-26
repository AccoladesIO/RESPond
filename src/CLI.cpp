#include "CLI.h"
#include "ResponseParser.h"
#include <string>
#include <iostream>

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

CLI::CLI(const std::string& host, int port)
    : redisClient(host, port) {}

void CLI::run() {
    if (!redisClient.connectToServer()) {
        std::cerr << "Failed to connect to Redis server." << std::endl;
        return;
    }

    std::cout << "Connected to Redis server at " << redisClient.getSocketFD() << "\n";
    std::string host = "127.0.0.1";

    while (true) {
        std::cout << host << ":" << redisClient.getSocketFD() << "> ";
        std::cout.flush();

        std::string line;
        std::getline(std::cin, line);
        line = trim(line);

        if (line.empty()) continue;
        if (line == "exit" || line == "quit") {
            std::cout << "Exiting CLI.\n";
            break;
        }
        if (line == "help") {
            std::cout << "Available commands:\n";
            std::cout << "  help - Show this help message\n";
            std::cout << "  exit or quit - Exit the CLI\n";
            std::cout << "  clear - Clear the screen\n";
            std::cout << "  history - Show command history\n";
            continue;
        }
        if (line == "history") {
            std::cout << "Command history:\n";
            for (size_t i = 0; i < history.size(); ++i) {
                std::cout << i + 1 << ": " << history[i] << "\n";
            }
            continue;
        }

        history.push_back(line); // save command

        std::vector<std::string> args = CommandHandler::parseCommand(line);
        if (args.empty()) continue;

        std::string command = CommandHandler::buildRESPCommand(args);
        if (!redisClient.sendCommand(command)) {
            std::cerr << "Failed to send command to Redis server.\n";
            break;
        }

        std::string response = ResponseParser::parseResponse(redisClient.getSocketFD());
        std::cout << response << "\n";
    }

    redisClient.disconnectFromServer();
}
