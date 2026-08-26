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

    // RESP3 additions
    static std::string parseBoolean(int socketFd);
    static std::string parseNull(int socketFd);
    static std::string parseDouble(int socketFd);
    static std::string parseBigNumber(int socketFd);
    static std::string parseVerbatimString(int socketFd);
    static std::string parseMap(int socketFd);
    static std::string parseSet(int socketFd);
    static std::string parsePush(int socketFd);
    static std::string parseAttribute(int socketFd);
};

#endif
