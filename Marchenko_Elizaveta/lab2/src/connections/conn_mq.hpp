#pragma once
#include <chrono>
#include <filesystem>
#include <mqueue.h>

#include "Connection.hpp"

class conn_mq final : public Connection {
    std::string name;
    mq_attr attr{};
    mqd_t mq{};

public:
    conn_mq(int fromPid, int toPid, bool isHost);
    bool Read(std::string &) override;
    bool Write(const std::string &) override;

    ~conn_mq() override;

    conn_mq() = default;
    conn_mq(const conn_mq&) = default;
    conn_mq& operator = (const conn_mq&) = default;
    conn_mq(conn_mq &&) = default;
    conn_mq &operator=(conn_mq &&) = default;
};


