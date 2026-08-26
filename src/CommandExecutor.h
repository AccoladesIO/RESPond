#ifndef COMMAND_EXECUTOR_H
#define COMMAND_EXECUTOR_H

#include <string>
#include <vector>

namespace CommandExecutor {
    bool execute(const std::string& host,
                 int port,
                 const std::vector<std::string>& args,
                 int timeout,
                 int retries,
                 bool verbose);
}


#endif