#include "CLI.h"
#include <string>

static std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) {
        return ""; // String is all whitespace
    }
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

CLI::CLI(const std::string& host, int port)
    : redisClient(host, port) {
}
void CLI::run() {
    if (!redisClient.connectToServer()) {
        std::cerr << "Failed to connect to Redis server." << std::endl;
        return;
    }
    
    std::cout << "Connected to Redis server at " << redisClient.getSocketFD() << "\n";
    std::string host = "127.0.0.1";

    while(true){
        std::cout << host << ":" << redisClient.getSocketFD() << "> ";

        std::cout.flush();
        std:: string line;
        std::getline(std::cin, line);

        line = trim(line);
        if (line.empty()) {
            continue;
        }
        if (line == "exit" || line == "quit") {
            std::cout << "Exiting CLI." << "\n";
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

        // split commands
        std::vector<std::string> args = CommandHandler::parseCommand(line);

        if (args.empty()){
            continue;
        };

        // for(const auto &arg: args){
        //     std::cout << arg << "\n";
        // }
        std::string command = CommandHandler::buildRESPCommand(args);
        if (!redisClient.sendCommand(command)) {
            std::cerr << "Failed to send command to Redis server." << "\n";
            break;
        }
    }
}