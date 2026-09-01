#include "ResponseParser.h"
#include "Utils.h"
#include <iostream>
#include <sstream>
#include <sys/socket.h>
#include <unistd.h>

std::string ResponseParser::escapeJson(const std::string &str)
{
    std::ostringstream oss;
    for (char c : str)
    {
        switch (c)
        {
        case '"':
            oss << "\\\"";
            break;
        case '\\':
            oss << "\\\\";
            break;
        case '\b':
            oss << "\\b";
            break;
        case '\f':
            oss << "\\f";
            break;
        case '\n':
            oss << "\\n";
            break;
        case '\r':
            oss << "\\r";
            break;
        case '\t':
            oss << "\\t";
            break;
        default:
            if ('\x00' <= c && c <= '\x1f')
            {
                oss << "\\u00" << ((c < 16) ? "0" : "") << std::hex << static_cast<int>(c);
            }
            else
            {
                oss << c;
            }
            break;
        }
    }
    return oss.str();
}

static bool looksLikeInfoReply(const std::string &text)
{
    return text.rfind("# ", 0) == 0 && text.find('\n') != std::string::npos;
}

static std::string parseInfoToJson(const std::string &raw)
{
    std::istringstream stream(raw);
    std::string line;
    std::ostringstream oss;
    std::string currentSection = "";
    bool firstSection = true;
    bool firstField = true;

    oss << "{";

    while (std::getline(stream, line))
    {
        while (!line.empty() && (line.back() == '\r' || line.back() == '\n' || line.back() == ' '))
        {
            line.pop_back();
        }
        if (line.empty())
            continue;

        if (line[0] == '#')
        {
            if (!currentSection.empty())
            {
                oss << "}";
            }
            currentSection = line.substr(2);
            if (!firstSection)
                oss << ", ";
            oss << "\"" << currentSection << "\": {";
            firstSection = false;
            firstField = true;
            continue;
        }

        size_t colon = line.find(':');
        if (colon != std::string::npos)
        {
            std::string key = line.substr(0, colon);
            std::string val = line.substr(colon + 1);

            if (!firstField)
                oss << ", ";
            oss << "\"" << key << "\": \"" << val << "\"";
            firstField = false;
        }
    }

    if (!currentSection.empty())
    {
        oss << "}";
    }
    oss << "}";
    return oss.str();
}

std::string ResponseParser::parseSimpleString(int socketFd)
{
    return "\"" + escapeJson(Utils::readLine(socketFd)) + "\"";
}

std::string ResponseParser::parseSimpleError(int socketFd)
{
    return "{\"error\": \"" + escapeJson(Utils::readLine(socketFd)) + "\"}";
}

std::string ResponseParser::parseIntegers(int socketFd)
{
    return Utils::readLine(socketFd);
}

std::string ResponseParser::parseBulkString(int socketFd)
{
    std::string lenStr = Utils::readLine(socketFd);
    int len = std::stoi(lenStr);
    if (len == -1)
        return "null";

    std::string value;
    value.resize(len);
    size_t totalRead = 0;
    while (totalRead < static_cast<size_t>(len))
    {
        ssize_t r = recv(socketFd, &value[totalRead], len - totalRead, 0);
        if (r <= 0)
            return "{\"error\": \"Failed to read bulk string\"}";
        totalRead += r;
    }

    char crlf[2];
    recv(socketFd, crlf, 2, 0);

    if (looksLikeInfoReply(value))
    {
        return parseInfoToJson(value);
    }

    return "\"" + escapeJson(value) + "\"";
}

std::string ResponseParser::parseArray(int socketFd)
{
    std::string countStr = Utils::readLine(socketFd);
    int count = std::stoi(countStr);
    if (count == -1)
        return "null";

    std::ostringstream oss;
    oss << "[";
    for (int i = 0; i < count; ++i)
    {
        oss << parseRawResponse(socketFd);
        if (i < count - 1)
            oss << ", ";
    }
    oss << "]";
    return oss.str();
}

std::string ResponseParser::parseBoolean(int socketFd)
{
    char val;
    Utils::readChar(socketFd, val);
    Utils::readLine(socketFd);
    return (val == 't') ? "true" : "false";
}

std::string ResponseParser::parseNull(int socketFd)
{
    Utils::readLine(socketFd);
    return "null";
}

std::string ResponseParser::parseDouble(int socketFd)
{
    return Utils::readLine(socketFd);
}

std::string ResponseParser::parseBigNumber(int socketFd)
{
    return "\"" + escapeJson(Utils::readLine(socketFd)) + "\"";
}

std::string ResponseParser::parseVerbatimString(int socketFd)
{
    std::string lenStr = Utils::readLine(socketFd);
    int len = std::stoi(lenStr);
    if (len == -1)
        return "null";

    std::string raw;
    raw.resize(len);
    size_t totalRead = 0;
    while (totalRead < static_cast<size_t>(len))
    {
        ssize_t r = recv(socketFd, &raw[totalRead], len - totalRead, 0);
        if (r <= 0)
            return "{\"error\": \"Failed to read verbatim string\"}";
        totalRead += r;
    }

    char crlf[2];
    recv(socketFd, crlf, 2, 0);

    std::string content = (len > 4) ? raw.substr(4) : raw;
    return "\"" + escapeJson(content) + "\"";
}

std::string ResponseParser::parseMap(int socketFd)
{
    std::string countStr = Utils::readLine(socketFd);
    int count = std::stoi(countStr);
    if (count == -1)
        return "null";

    std::ostringstream oss;
    oss << "{";
    for (int i = 0; i < count; ++i)
    {
        std::string key = parseRawResponse(socketFd);
        if (key.front() != '"')
        {
            key = "\"" + escapeJson(key) + "\"";
        }
        std::string value = parseRawResponse(socketFd);
        oss << key << ": " << value;
        if (i < count - 1)
            oss << ", ";
    }
    oss << "}";
    return oss.str();
}

std::string ResponseParser::parseSet(int socketFd)
{
    return parseArray(socketFd);
}

std::string ResponseParser::parsePush(int socketFd)
{
    std::string countStr = Utils::readLine(socketFd);
    int count = std::stoi(countStr);
    if (count == -1)
        return "null";

    std::ostringstream oss;
    oss << "{\"push\": [";
    for (int i = 0; i < count; ++i)
    {
        oss << parseRawResponse(socketFd);
        if (i < count - 1)
            oss << ", ";
    }
    oss << "]}";
    return oss.str();
}

std::string ResponseParser::parseAttribute(int socketFd)
{
    std::string countStr = Utils::readLine(socketFd);
    int count = std::stoi(countStr);
    if (count == -1)
        return "null";

    std::ostringstream oss;
    oss << "{\"attributes\": {";
    for (int i = 0; i < count; ++i)
    {
        std::string key = parseRawResponse(socketFd);
        if (key.front() != '"')
        {
            key = "\"" + escapeJson(key) + "\"";
        }
        std::string value = parseRawResponse(socketFd);
        oss << key << ": " << value;
        if (i < count - 1)
            oss << ", ";
    }
    oss << "}, \"data\": " << parseRawResponse(socketFd) << "}";
    return oss.str();
}

std::string ResponseParser::parseRawResponse(int socketFd)
{
    char prefix;
    if (!Utils::readChar(socketFd, prefix))
    {
        return "{\"error\": \"No response or connection closed\"}";
    }

    switch (prefix)
    {
    case '+':
        return parseSimpleString(socketFd);
    case '-':
        return parseSimpleError(socketFd);
    case ':':
        return parseIntegers(socketFd);
    case '$':
        return parseBulkString(socketFd);
    case '*':
        return parseArray(socketFd);
    case '#':
        return parseBoolean(socketFd);
    case '_':
        return parseNull(socketFd);
    case ',':
        return parseDouble(socketFd);
    case '(':
        return parseBigNumber(socketFd);
    case '=':
        return parseVerbatimString(socketFd);
    case '%':
        return parseMap(socketFd);
    case '~':
        return parseSet(socketFd);
    case '>':
        return parsePush(socketFd);
    case '|':
        return parseAttribute(socketFd);
    default:
        return "{\"error\": \"Unknown response type: " + std::string(1, prefix) + "\"}";
    }
}

std::string ResponseParser::parseResponse(int socketFd, bool pretty)
{
    std::string rawJson = parseRawResponse(socketFd);
    return pretty ? Utils::formatPrettyJson(rawJson) : rawJson;
}