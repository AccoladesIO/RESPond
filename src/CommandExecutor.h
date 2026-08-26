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

    bool executePipeline(const std::string& host,
                         int port,
                         const std::vector<std::vector<std::string>>& commands,
                         int timeout,
                         int retries,
                         bool verbose);

    bool executeScript(const std::string& host,
                   int port,
                   const std::string& scriptPath,
                   int timeout,
                   int retries,
                   bool verbose);
    bool executeInlineScript(const std::string& host,
                         int port,
                         const std::string& scriptBody,
                         int timeout,
                         int retries,
                         bool verbose);

}

#endif
