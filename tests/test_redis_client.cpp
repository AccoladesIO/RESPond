#include <gtest/gtest.h>
#include "RedisClient.h"

#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <string>
#include <thread>

// ===========================================================================
// RedisClient talks to a real socket, so these are small integration tests
// against a throwaway TCP server bound to an ephemeral port on 127.0.0.1.
// No external Redis needed.
// ===========================================================================

namespace {

// A minimal one-shot loopback server. Binds to port 0 (kernel picks a free
// port), accepts a single connection, and stores whatever bytes it receives.
class LoopbackServer {
public:
    LoopbackServer() {
        listenFd_ = ::socket(AF_INET, SOCK_STREAM, 0);
        EXPECT_NE(listenFd_, -1);

        int yes = 1;
        ::setsockopt(listenFd_, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));

        sockaddr_in addr{};
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
        addr.sin_port = 0;  // ephemeral

        EXPECT_EQ(::bind(listenFd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)), 0);
        EXPECT_EQ(::listen(listenFd_, 1), 0);

        socklen_t len = sizeof(addr);
        EXPECT_EQ(::getsockname(listenFd_, reinterpret_cast<sockaddr*>(&addr), &len), 0);
        port_ = ntohs(addr.sin_port);
    }

    ~LoopbackServer() {
        if (listenFd_ != -1) ::close(listenFd_);
        if (connFd_ != -1) ::close(connFd_);
    }

    int port() const { return port_; }

    // Accept one client and read up to `expectedBytes` into received_.
    void acceptAndRead(size_t expectedBytes) {
        connFd_ = ::accept(listenFd_, nullptr, nullptr);
        if (connFd_ == -1) return;
        char buf[1024];
        while (received_.size() < expectedBytes) {
            ssize_t n = ::recv(connFd_, buf, sizeof(buf), 0);
            if (n <= 0) break;
            received_.append(buf, static_cast<size_t>(n));
        }
    }

    const std::string& received() const { return received_; }

private:
    int listenFd_ = -1;
    int connFd_ = -1;
    int port_ = 0;
    std::string received_;
};

// Find a port that is bound then immediately released, so a later connect()
// to it is refused. Reliable on the loopback interface.
int closedPort() {
    int fd = ::socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr{};
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    addr.sin_port = 0;
    ::bind(fd, reinterpret_cast<sockaddr*>(&addr), sizeof(addr));
    socklen_t len = sizeof(addr);
    ::getsockname(fd, reinterpret_cast<sockaddr*>(&addr), &len);
    int port = ntohs(addr.sin_port);
    ::close(fd);
    return port;
}

}  // namespace

TEST(RedisClientConnect, ConnectsToLoopbackServer) {
    LoopbackServer server;
    std::thread serverThread([&] { server.acceptAndRead(0); });

    RedisClient client("127.0.0.1", server.port());
    EXPECT_TRUE(client.connectToServer());
    EXPECT_NE(client.getSocketFD(), -1);

    serverThread.join();
}

TEST(RedisClientConnect, FailsWhenNothingIsListening) {
    RedisClient client("127.0.0.1", closedPort());
    EXPECT_FALSE(client.connectToServer());
    EXPECT_EQ(client.getSocketFD(), -1);
}

TEST(RedisClientSend, SendsExactBytesOverTheWire) {
    const std::string payload = "*1\r\n$4\r\nPING\r\n";

    LoopbackServer server;
    std::thread serverThread([&] { server.acceptAndRead(payload.size()); });

    RedisClient client("127.0.0.1", server.port());
    ASSERT_TRUE(client.connectToServer());
    EXPECT_TRUE(client.sendCommand(payload));

    serverThread.join();
    EXPECT_EQ(server.received(), payload);
}

TEST(RedisClientSend, ReturnsFalseWhenNotConnected) {
    RedisClient client("127.0.0.1", 6379);  // never connected
    EXPECT_FALSE(client.sendCommand("*1\r\n$4\r\nPING\r\n"));
}