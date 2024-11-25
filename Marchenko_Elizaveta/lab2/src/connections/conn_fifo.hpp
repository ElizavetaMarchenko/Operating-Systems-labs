#pragma once
#include "Connection.hpp"


class conn_fifo final : public Connection {
public:
    conn_fifo(int fromPid, int toPid, bool isHost);

    bool Read(std::string& msg) override;
    bool Write(const std::string& msg) override;

    ~conn_fifo() override;

    conn_fifo() = default;
    conn_fifo(const conn_fifo&) = default;
    conn_fifo& operator = (const conn_fifo&) = default;
    conn_fifo(conn_fifo &&) = default;
    conn_fifo &operator=(conn_fifo &&) = default;

private:
    bool _isHost{};
    std::string _fifoName;
    int _fileDescr = -1;
};


