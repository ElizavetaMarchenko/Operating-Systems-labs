#pragma once
#include <iostream>

namespace config{
    inline std::string host_pid_path = "host.pid";
    inline std::string global_semaphore_path = "../src/global_semaphore";

    inline std::string create_path(const int host_pid, const int client_pid) {
        return std::to_string(host_pid) + "_" + std::to_string(client_pid);
    }
}
