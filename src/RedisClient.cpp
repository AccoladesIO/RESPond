#include "RedisClient.h"
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

RedisClient::RedisClient(const std::string &host, int port)
    : host(host), port(port), socketfd(-1) {}

RedisClient::~RedisClient()
{
    disconnectFromServer();
}

void RedisClient::closeServerConnection()
{
    if (socketfd != -1)
    {
        close(socketfd);
        socketfd = -1;
    }
}

void RedisClient::setTimeout(int seconds)
{
    timeoutSeconds = seconds;
}

void RedisClient::setRetries(int count)
{
    retryCount = count;
}

bool RedisClient::connectToServer()
{
    struct addrinfo hints, *res;
    std::memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    std::string portStr = std::to_string(port);
    int status = getaddrinfo(host.c_str(), portStr.c_str(), &hints, &res);
    if (status != 0)
    {
        std::cerr << "getaddrinfo error: " << gai_strerror(status) << "\n";
        return false;
    }

    bool connected = false;
    for (int attempt = 1; attempt <= retryCount && !connected; ++attempt)
    {
        for (auto p = res; p != nullptr; p = p->ai_next)
        {
            socketfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol);
            if (socketfd == -1)
                continue;

            // Apply timeout
            struct timeval tv;
            tv.tv_sec = timeoutSeconds;
            tv.tv_usec = 0;
            setsockopt(socketfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
            setsockopt(socketfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));

            if (connect(socketfd, p->ai_addr, p->ai_addrlen) == 0)
            {
                connected = true;
                break;
            }

            close(socketfd);
            socketfd = -1;
        }

        if (!connected)
        {
            std::cerr << "Connection attempt " << attempt << " failed.\n";
            if (attempt < retryCount)
            {
                int waitTime = (1 << (attempt - 1));
                std::cerr << "Retrying in " << waitTime << "s...\n";
                sleep(waitTime);
            }
        }
    }

    freeaddrinfo(res);

    if (!connected)
    {
        std::cerr << "Failed to connect to " << host << ":" << port
                  << " after " << retryCount << " attempts.\n";
        return false;
    }

    return true;
}

bool RedisClient::sendCommand(const std::string &command)
{
    if (socketfd == -1)
    {
        return false;
    }

    ssize_t sent = send(socketfd, command.c_str(), command.length(), 0);
    return sent == (ssize_t)command.length();
}

void RedisClient::disconnectFromServer()
{
    closeServerConnection();
}

int RedisClient::getSocketFD() const
{
    return socketfd;
}
