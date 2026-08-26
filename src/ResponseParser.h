#ifndef RESPONSEPARSER_H
#define RESPONSEPARSER_H

#include <string>

class ResponseParser {
public:
    static std::string parseResponse(int socketFd);

private:
    static std::string parseSimpleString(int socketFd);
    static std::string parseSimpleError(int socketFd);
    static std::string parseIntegers(int socketFd);
    static std::string parseBulkString(int socketFd);
    static std::string parseArray(int socketFd);
    static std::string parseNull(int socketFd);
};

#endif