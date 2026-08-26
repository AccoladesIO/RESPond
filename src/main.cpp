#include <iostream>
#include <string>
#include <vector>
#include "CLI.h"
#include "CommandExecutor.h"
#include "Config.h"

int main(int argc, char *argv[])
{
    std::string host = "127.0.0.1";
    int port = 6379;
    int timeout = 30;
    bool verbose = false;
    int retries = 3;
    std::vector<std::string> commandArgs;

    int i = 1;
    while (i < argc)
    {
        std::string arg = argv[i];
        if (arg == "-h" && i + 1 < argc)
        {
            host = argv[i + 1];
            i += 2;
        }
        else if (arg == "-p" && i + 1 < argc)
        {
            port = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "-t" && i + 1 < argc)
        {
            timeout = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "-v")
        {
            verbose = true;
            i++;
        }
        else if (arg == "-r" && i + 1 < argc)
        {
            retries = std::stoi(argv[i + 1]);
            i += 2;
        }
        else if (arg == "--script" && i + 1 < argc)
        {
            std::string scriptPath = argv[++i];
            CommandExecutor::executeScript(host, port, scriptPath, timeout, retries, verbose);
            return 0;
        }
        else if (arg == "--inline" && i + 1 < argc)
        {
            std::string scriptBody = argv[++i];
            CommandExecutor::executeInlineScript(host, port, scriptBody, timeout, retries, verbose);
            return 0;
        }
        else if (arg[0] == '-')
        {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
        else
        {
            while (i < argc)
            {
                commandArgs.push_back(argv[i]);
                i++;
            }
        }
    }
    Config::save(host, port);

    if (!commandArgs.empty())
    {
        if (verbose)
        {
            std::cout << "Executing one-shot command on " << host << ":" << port << "\n";
        }
        return CommandExecutor::execute(host, port, commandArgs, timeout, retries, verbose) ? 0 : 1;
    }

    if (verbose)
    {
        std::cout << "Starting interactive CLI on " << host << ":" << port
                  << " (timeout=" << timeout << "s)\n";
    }

    CLI cli(host, port);
    cli.run();
    return 0;
}