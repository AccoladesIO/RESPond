#include "CLI.h"
#include "CommandHandler.h"
#include "ResponseParser.h"
#include "Utils.h"
#include <string>
#include <iostream>
#include "AutoComplete.h"
#include <sstream>
#include "CommandExecutor.h"

CLI::CLI(const std::string &host, int port)
    : redisClient(host, port), host(host), port(port) {}

void CLI::run()
{
    if (!redisClient.connectToServer())
    {
        std::cerr << "Failed to connect to Redis server." << std::endl;
        return;
    }

    std::cout << "Connected to Redis server at " << redisClient.getSocketFD() << "\n";

    while (true)
    {
        std::cout << host << ":" << port << "> ";
        std::cout.flush();

        std::string line;
        if (!std::getline(std::cin, line))
        {
            break;
        }
        line = Utils::trim(line);

        std::string original = line;
        line = Autocomplete::complete(line);

        // If multiple suggestions exist, show them
        auto sugg = Autocomplete::suggestions(original);
        if (sugg.size() > 1)
        {
            std::cout << "Did you mean: ";
            for (const auto &s : sugg)
            {
                std::cout << s << " ";
            }
            std::cout << "\n";
        }

        if (line.empty())
            continue;
        if (line == "q")
            line = "quit";
        else if (line == "h")
            line = "help";
        else if (line == "c")
            line = "clear";
        else if (line == "hist")
            line = "history";
        else if (line == "ch")
            line = "clear-history";

        if (line == "exit" || line == "quit")
        {
            std::cout << "Exiting CLI.\n";
            break;
        }
        if (line == "help")
        {
            std::cout << "Available commands:\n";
            std::cout << "  help (h) - Show this help message\n";
            std::cout << "  exit or quit (q) - Exit the CLI\n";
            std::cout << "  clear (c) - Clear the screen\n";
            std::cout << "  history (hist) - Show command history\n";
            std::cout << "  clear-history (ch) - Clear stored command history\n";
            continue;
        }
        if (line == "history")
        {
            std::cout << "Command history:\n";
            for (size_t i = 0; i < history.size(); ++i)
            {
                std::cout << i + 1 << ": " << history[i] << "\n";
            }
            continue;
        }
        if (line == "clear-history")
        {
            history.clear();
            std::cout << "History cleared.\n";
            continue;
        }
        if (line == "clear")
        {
            std::cout << "\033[2J\033[H"; // clear screen + move cursor to top
            continue;
        }
        if (line.rfind("PIPELINE", 0) == 0)
        {
            std::string pipelineBody = line.substr(8);
            std::stringstream ss(pipelineBody);
            std::string segment;
            std::vector<std::vector<std::string>> pipelineCommands;

            while (std::getline(ss, segment, ';'))
            {
                segment = Utils::trim(segment);
                if (segment.empty())
                    continue;
                std::vector<std::string> cmdArgs = CommandHandler::parseCommand(segment);
                if (!cmdArgs.empty())
                    pipelineCommands.push_back(cmdArgs);
            }

            if (!pipelineCommands.empty())
            {
                CommandExecutor::executePipeline(host, port, pipelineCommands, 5, 3, false);
            }
            continue;
        }

        history.push_back(line);

        std::vector<std::string> args = CommandHandler::parseCommand(line);
        if (args.empty())
            continue;

        std::string command = CommandHandler::buildRESPCommand(args);
        if (!redisClient.sendCommand(command))
        {
            std::cerr << "Failed to send command to Redis server.\n";
            break;
        }

        std::string response = ResponseParser::parseResponse(redisClient.getSocketFD());
        std::cout << response << "\n";
    }

    redisClient.disconnectFromServer();
}