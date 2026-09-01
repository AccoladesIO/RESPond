#ifndef RESPONSE_PARSER_H
#define RESPONSE_PARSER_H

#include <string>

class ResponseParser {
public:
    static std::string parseResponse(int socketFd, bool pretty = true);

private:
    static std::string parseRawResponse(int socketFd);
    static std::string parseSimpleString(int socketFd);
    static std::string parseSimpleError(int socketFd);
    static std::string parseIntegers(int socketFd);
    static std::string parseBulkString(int socketFd);
    static std::string parseArray(int socketFd);
    static std::string parseBoolean(int socketFd);
    static std::string parseNull(int socketFd);
    static std::string parseDouble(int socketFd);
    static std::string parseBigNumber(int socketFd);
    static std::string parseVerbatimString(int socketFd);
    static std::string parseMap(int socketFd);
    static std::string parseSet(int socketFd);
    static std::string parsePush(int socketFd);
    static std::string parseAttribute(int socketFd);
    static std::string escapeJson(const std::string& str);
};

#endif