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
    void setTimeout(int seconds); 
    void setRetries(int count);

private:
    std::string host;
    int port;
    int socketfd;
    int timeoutSeconds = 30;
    int retryCount = 3; 
};

#endif // REDIS_CLIENT_H