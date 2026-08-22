#include "RedisClient.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

RedisClient::RedisClient(const std::string &host, int port)
    : host(host), port(port), socketfd(-1) {}

RedisClient::~RedisClient() {
    disconnectFromServer();
}

void RedisClient::closeServerConnection() {
    if (socketfd != -1) {
        close(socketfd);
        socketfd = -1;
    }
}

bool RedisClient::connectToServer() {
    struct addrinfo hints, *res;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    std::string portStr = std::to_string(port);
    int status = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res);
    if (status != 0) {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << "\n";
        return false;
    }

    for (auto p = res; p != nullptr; p = p->ai_next) {
        socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
        if (socketfd == -1) {
            continue;
        }

        if (connect(socketfd, p->ai_addr, p->ai_addrlen) == 0) {
            break;
        }
        close(socketfd);
        socketfd = -1;
    }
    freeaddrinfo(res);

    if (socketfd == -1) {
        std::cerr << "Failed to connect to " << host << ":" << port << "\n";
        return false;
    }

    return true;
}

bool RedisClient::sendCommand(const std::string& command) {
    if (socketfd == -1) {
        return false;
    }

    ssize_t sent = send(socketfd, command.c_str(), command.length(), 0);
    return sent == (ssize_t)command.length();
}


void RedisClient::disconnectFromServer() {
    closeServerConnection();
}

int RedisClient::getSocketFD() const {
    return socketfd;
}
