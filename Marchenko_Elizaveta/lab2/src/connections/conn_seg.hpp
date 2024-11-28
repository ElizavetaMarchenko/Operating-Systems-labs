#pragma once
#include "Connection.hpp"

class conn_seg : public Connection {
public:
    conn_seg(int fromPid, int toPid, bool isHost);

    bool Read(std::string& msg) override;
    bool Write(const std::string& msg) override;

    ~conn_seg() override;

    conn_seg() = default;
    conn_seg(const conn_seg&) = default;
    conn_seg& operator = (const conn_seg&) = default;
    conn_seg(conn_seg &&) = default;
    conn_seg &operator=(conn_seg &&) = default;

private:

    bool _isHost;
    void *_segptr = nullptr;
    int _shmid = -1;
    std::string name;
};


