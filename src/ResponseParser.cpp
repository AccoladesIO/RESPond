#include "ResponseParser.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>


std::string ResponseParser::parseResponse(int socketFd) {
    char prefix;
    if (!Utils::readChar(socketFd, prefix)) {
        return "(Error) No response or connection closed";
    }

    switch (prefix) {
        case '+': return parseSimpleString(socketFd);
        case '-': return parseSimpleError(socketFd);
        case ':': return parseIntegers(socketFd);
        case '$': return parseBulkString(socketFd);
        case '*': return parseArray(socketFd);

        // RESP3
        case '#': return parseBoolean(socketFd);
        case '_': return parseNull(socketFd);
        case ',': return parseDouble(socketFd);
        case '(': return parseBigNumber(socketFd);
        case '=': return parseVerbatimString(socketFd);
        case '%': return parseMap(socketFd);
        case '~': return parseSet(socketFd);
        case '>': return parsePush(socketFd);
        case '|': return parseAttribute(socketFd);

        default:  return "(Error) Unknown response type";
    }
}


std::string ResponseParser::parseSimpleString(int socketFd) {
    return Utils::readLine(socketFd);
}

std::string ResponseParser::parseSimpleError(int socketFd) {
    return "(Error) " + Utils::readLine(socketFd);
}

std::string ResponseParser::parseIntegers(int socketFd) {
    return Utils::readLine(socketFd);
}

std::string ResponseParser::parseBulkString(int socketFd) {
    std::string lenStr = Utils::readLine(socketFd);
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
    std::string countStr = Utils::readLine(socketFd);
    int count = std::stoi(countStr);
    if (count == -1) return "(nil)";

    std::ostringstream oss;
    for (int i = 0; i < count; ++i) {
        oss << i << ") " << parseResponse(socketFd) << "\n";
    }
    return oss.str();
}

std::string ResponseParser::parseBoolean(int socketFd) {
    char val;
    Utils::readChar(socketFd, val);
    return (val == 't') ? "(true)" : "(false)";
}

std::string ResponseParser::parseNull(int socketFd) {
    return "(null)";
}

std::string ResponseParser::parseDouble(int socketFd) {
    return Utils::readLine(socketFd);
}
 
std::string ResponseParser::parseBigNumber(int socketFd) {
    return Utils::readLine(socketFd);
}

std::string ResponseParser::parseVerbatimString(int socketFd) {
    std::string header = Utils::readLine(socketFd); // e.g. "txt:Hello"
    return "(verbatim) " + header;
}

std::string ResponseParser::parseMap(int socketFd) {
    std::string countStr = Utils::readLine(socketFd);
    int count = std::stoi(countStr);
    if (count == -1) return "(nil)";

    std::ostringstream oss;
    oss << "Map[" << count << "]:\n";
    for (int i = 0; i < count; ++i) {
        std::string key = parseResponse(socketFd);
        std::string value = parseResponse(socketFd);
        oss << "  " << key << " => " << value << "\n";
    }
    return oss.str();
}

std::string ResponseParser::parseSet(int socketFd) {
    std::string countStr = Utils::readLine(socketFd);
    int count = std::stoi(countStr);
    if (count == -1) return "(nil)";

    std::ostringstream oss;
    oss << "Set[" << count << "]:\n";
    for (int i = 0; i < count; ++i) {
        oss << "  " << parseResponse(socketFd) << "\n";
    }
    return oss.str();
}

std::string ResponseParser::parsePush(int socketFd) {
    std::string countStr = Utils::readLine(socketFd);
    int count = std::stoi(countStr);
    if (count == -1) return "(nil)";

    std::ostringstream oss;
    oss << "Push[" << count << "]:\n";
    for (int i = 0; i < count; ++i) {
        oss << "  " << parseResponse(socketFd) << "\n";
    }
    return oss.str();
}

std::string ResponseParser::parseAttribute(int socketFd) {
    std::string countStr = Utils::readLine(socketFd);
    int count = std::stoi(countStr);
    if (count == -1) return "(nil)";

    std::ostringstream oss;
    oss << "Attributes[" << count << "]:\n";
    for (int i = 0; i < count; ++i) {
        std::string key = parseResponse(socketFd);
        std::string value = parseResponse(socketFd);
        oss << "  " << key << " => " << value << "\n";
    }
    return oss.str();
}

