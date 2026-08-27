#include "Utils.h"
#include <sys/socket.h>
#include <unistd.h>
#include <sstream>

namespace Utils
{

    std::string trim(const std::string &str)
    {
        size_t first = str.find_first_not_of(" \t\n\r");
        if (first == std::string::npos)
            return "";
        size_t last = str.find_last_not_of(" \t\n\r");
        return str.substr(first, (last - first + 1));
    }

    bool readChar(int sockfd, char &c)
    {
        ssize_t r = recv(sockfd, &c, 1, 0);
        return (r == 1);
    }

    std::string readLine(int sockfd)
    {
        std::string line;
        char c;
        while (readChar(sockfd, c))
        {
            if (c == '\r')
                continue;
            if (c == '\n')
                break;
            line.push_back(c);
        }
        return line;
    }

    std::string formatPrettyJson(const std::string &json, int indentSpaces)
    {
        std::ostringstream oss;
        int indent = 0;
        bool inQuotes = false;
        bool escape = false;

        for (size_t i = 0; i < json.size(); ++i)
        {
            char c = json[i];

            if (escape)
            {
                oss << c;
                escape = false;
                continue;
            }

            if (c == '\\')
            {
                oss << c;
                escape = true;
                continue;
            }

            if (c == '"')
            {
                inQuotes = !inQuotes;
                oss << c;
                continue;
            }

            if (inQuotes)
            {
                oss << c;
                continue;
            }

            if (c == '{' || c == '[')
            {
                oss << c;
                if (i + 1 < json.size() && ((c == '{' && json[i + 1] == '}') || (c == '[' && json[i + 1] == ']')))
                {
                    continue;
                }
                indent++;
                oss << "\n"
                    << std::string(indent * indentSpaces, ' ');
            }
            else if (c == '}' || c == ']')
            {
                if (i > 0 && ((c == '}' && json[i - 1] == '{') || (c == ']' && json[i - 1] == '[')))
                {
                    oss << c;
                    continue;
                }
                indent--;
                oss << "\n"
                    << std::string(indent * indentSpaces, ' ') << c;
            }
            else if (c == ',')
            {
                oss << ",\n"
                    << std::string(indent * indentSpaces, ' ');
            }
            else if (c == ':')
            {
                oss << ": ";
            }
            else if (c != ' ' && c != '\t' && c != '\n' && c != '\r')
            {
                oss << c;
            }
        }

        return oss.str();
    }

}