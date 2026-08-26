#include "Utils.h"
#include <sys/socket.h>
#include <unistd.h>

namespace Utils {

std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    if (first == std::string::npos) return "";
    size_t last = str.find_last_not_of(" \t\n\r");
    return str.substr(first, (last - first + 1));
}

bool readChar(int sockfd, char &c) {
    ssize_t r = recv(sockfd, &c, 1, 0);
    return (r == 1);
}

std::string readLine(int sockfd) {
    std::string line;
    char c;
    while (readChar(sockfd, c)) {
        if (c == '\r') continue;
        if (c == '\n') break;
        line.push_back(c);
    }
    return line;
}

}