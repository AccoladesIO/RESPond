#include "Config.h"
#include <fstream>
#include <cstdlib>

bool Config::load(std::string& host, int& port) {
    std::string path = std::string(getenv("HOME")) + "/.respond.conf";
    std::ifstream in(path);
    if (!in.is_open()) return false;

    std::string h;
    int p;
    if (in >> h >> p) {
        host = h;
        port = p;
        return true;
    }
    return false;
}

void Config::save(const std::string& host, int port) {
    std::string path = std::string(getenv("HOME")) + "/.respond.conf";
    std::ofstream out(path);
    if (out.is_open()) {
        out << host << " " << port << "\n";
    }
}
