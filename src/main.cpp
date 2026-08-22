/*
1. Command line arguments parsing: The program should be able to accept command line arguments and parse them correctly. This includes handling flags, options, and positional arguments.
    -h <host> default: 127.0.0.1, -p <port> default: 8080, -t <timeout> default: 30, -v (verbose mode)
    if no args, launch interactive REPL mode

2. Object-Oriented Programming:
    RedisClient, CommandHandler, ResponseParser, CLI

3. Establish a TCP connection to a Redis server using the provided host and port. Implement error handling for connection failures.
    Berkeley sockets to open TCP connection, handle errors like connection refused, timeout, etc.
    IPv4 and IPv6 support

4. Parsing and command formatting (commandHandler):
    - Implement a command handler that takes user input, formats it according to the Redis protocol, and sends it to the server.
    - Handle different types of commands (e.g., GET, SET, DEL) and their respective arguments.

5. Handling server responses (responseParser):
    - Implement a response parser that reads the server's responses and formats them for display to the user.
    - Handle different types of responses (e.g., simple strings, errors, bulk strings, arrays).

6. Implement interacive REPL mode:
    Run loop: user input, accept command, send to server, receive response, display response, repeat until exit command.
    support: help, exit, clear, history, and other common REPL commands.

7. main.cpp: Implement the main function to initialize the Redis client, parse command line arguments, and start the REPL loop or execute a single command based on the provided arguments.


Socket programming: Use Berkeley sockets to establish a TCP connection to the Redis server. Implement error handling for connection failures, timeouts, and other socket-related issues. Support both IPv4 and IPv6 connections.
Protocol handling: Implement the Redis protocol for sending commands and receiving responses. This includes formatting commands according to the Redis protocol specification and parsing responses from the server.
OOP Principles: Use object-oriented programming principles to design the Redis client. Create classes for handling commands, parsing responses, and managing the connection to the Redis server. Ensure proper encapsulation, inheritance, and polymorphism where appropriate.
CLI development: Implement a command-line interface (CLI) that allows users to interact with the Redis client. This includes parsing command-line arguments, providing help messages, and supporting interactive REPL mode for executing commands.
*/

#include <iostream>
#include <string>
#include <vector>
#include "CLI.h"

int main(int argc, char* argv[]) {
    // Parse command line arguments
    std::string host = "127.0.0.1";
    int port = 6379;
    int timeout = 30;
    
    int i = 1;
    while (i < argc) {
        std::string arg = argv[i];
        if (arg == "-h" && i + 1 < argc){
            host = argv[i + 1];
            i += 2;
        } else if (arg == "-p" && i + 1 < argc) {
            port = std::stoi(argv[i + 1]);
            i += 2;
        } else if (arg == "-t" && i + 1 < argc) {
            timeout = std::stoi(argv[i + 1]);
            i += 2;
        } else if (arg == "-v") {
            // Enable verbose mode
            std::cout << "Verbose mode enabled." << std::endl;
            i++;
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            return 1;
        }
    }

    // Handle interactive REPL mode and one-time command execution based on parsed arguments
    CLI cli(host, port);
    cli.run();
    return 0;
}