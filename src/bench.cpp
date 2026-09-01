#include "RedisClient.h"
#include "ResponseParser.h"
#include <chrono>
#include <iostream>
#include <string>



void singleRequestBenchmark(RedisClient& client, int total) {
    const std::string payload = "*1\r\n$4\r\nPING\r\n";
    int fd = client.getSocketFD();
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < total; ++i) {
        if (!client.sendCommand(payload)) { std::cerr << "send failed @" << i << "\n"; return; }
        ResponseParser::parseResponse(fd, false);   // <-- drain the +PONG
    }
    auto end = std::chrono::high_resolution_clock::now();
    double s = std::chrono::duration<double>(end - start).count();
    std::cout << "[Single Request]  " << total << " reqs, " << s
              << " s, " << (long long)(total / s) << " req/sec\n";
}

void pipelineBenchmark(RedisClient& client, int total_batches, int batch_size) {
    std::string single = "*1\r\n$4\r\nPING\r\n", batch;
    for (int i = 0; i < batch_size; ++i) batch += single;
    int fd = client.getSocketFD();
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < total_batches; ++i) {
        if (!client.sendCommand(batch)) { std::cerr << "send failed @batch " << i << "\n"; return; }
        for (int j = 0; j < batch_size; ++j) ResponseParser::parseResponse(fd, false); // drain all replies
    }
    auto end = std::chrono::high_resolution_clock::now();
    double s = std::chrono::duration<double>(end - start).count();
    long long reqs = (long long)total_batches * batch_size;
    std::cout << "[Pipeline]        " << reqs << " reqs, " << s
              << " s, " << (long long)(reqs / s) << " req/sec\n";
}

int main() {
    RedisClient client("127.0.0.1", 6379);
    if (!client.connectToServer()) { std::cerr << "Failed to connect to Redis\n"; return 1; }
    singleRequestBenchmark(client, 100000);
    pipelineBenchmark(client, 1000, 1000);
    return 0;
}