#include "ResponseParser.h"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

static bool readChar(int sockfd, char &c) {
    ssize_t r = recv(sockfd, &c, 1, 0);
    return (r == 1);
}

static std::string readLine(int sockfd) {
    std::string line;
    char c;
    while (readChar(sockfd, c)) {
        if (c == '\r') continue;
        if (c == '\n') break;
        line.push_back(c);
    }
    return line;
}

std::string ResponseParser::parseResponse(int socketFd) {
    char prefix;
    if (!readChar(socketFd, prefix)) {
        return "(Error) No response or connection closed";
    }

    switch (prefix) {
        case '+': return parseSimpleString(socketFd);
        case '-': return parseSimpleError(socketFd);
        case ':': return parseIntegers(socketFd);
        case '$': return parseBulkString(socketFd);
        case '*': return parseArray(socketFd);
        default:  return "(Error) Unknown response type";
    }
}

std::string ResponseParser::parseSimpleString(int socketFd) {
    return readLine(socketFd);
}

std::string ResponseParser::parseSimpleError(int socketFd) {
    return "(Error) " + readLine(socketFd);
}

std::string ResponseParser::parseIntegers(int socketFd) {
    return readLine(socketFd);
}

std::string ResponseParser::parseBulkString(int socketFd) {
    std::string lenStr = readLine(socketFd);
    int len = std::stoi(lenStr);
    if (len == -1) return "(nil)";

    std::string value;
    value.resize(len);
    ssize_t r = recv(socketFd, &value[0], len, 0);
    if (r != len) return "(Error) Failed to read bulk string";

    char crlf[2];
    recv(socketFd, crlf, 2, 0);

    return value;
}

std::string ResponseParser::parseArray(int socketFd) {
    std::string countStr = readLine(socketFd);
    int count = std::stoi(countStr);
    if (count == -1) return "(nil)";

    std::ostringstream oss;
    oss << "Array[" << count << "]:\n";
    for (int i = 0; i < count; ++i) {
        oss << "  " << parseResponse(socketFd) << "\n";
    }
    return oss.str();
}
