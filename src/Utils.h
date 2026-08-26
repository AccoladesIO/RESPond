#ifndef UTILS_H
#define UTILS_H

#include <string>

namespace Utils {
    std::string trim(const std::string& str);
    bool readChar(int sockfd, char &c);
    std::string readLine(int sockfd);
    std::string formatPrettyJson(const std::string& json, int indentSpaces = 2);
    
}

#endif