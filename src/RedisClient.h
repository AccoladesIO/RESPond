#ifndef REDIS_CLIENT_H
#define REDIS_CLIENT_H
#include <string>
#include <iostream>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#



class RedisClient {
public:
    RedisClient(const std::string& host, int port);
    ~RedisClient();

    bool connectToServer();
    void disconnectFromServer();
    bool sendCommand(const std::string& command);
    void closeServerConnection();
    int getSocketFD() const;

private:
    std::string host;
    int port;
    int socketfd;
};

#endif // REDIS_CLIENT_H