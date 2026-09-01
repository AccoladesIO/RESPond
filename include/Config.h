#ifndef CONFIG_H
#define CONFIG_H

#include <string>

class Config {
public:
    static bool load(std::string& host, int& port);
    static void save(const std::string& host, int port);
};

#endif // CONFIG_H
