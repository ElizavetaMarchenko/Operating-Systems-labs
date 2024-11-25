#include "conn_mq.hpp"
#include <sys/syslog.h>

#include <sys/un.h>
#include "../config.hpp"
using namespace config;

conn_mq::conn_mq(int fromPid, int toPid, bool isHost) {
    const std::string name = create_path(fromPid, toPid) + "mq";
    if (isHost)
    {
        mq_attr attr;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = 1024;
        attr.mq_curmsgs = 0;
        attr.mq_flags = 0;
        mq = mq_open(name.c_str(), O_CREAT | O_RDWR, S_IRWXU | S_IRWXG | S_IRWXO, &attr);
    }
    else
    {
        mq = mq_open(name.c_str(), O_RDWR);
    }

    if (mq < 0)
    {
        syslog(LOG_ERR, "ERROR: failed to create");
        throw std::runtime_error("failed to create MQ");
    }
}

bool conn_mq::Read(std::string &msg) {
    if (mq_receive(mq, (char *)(&msg), sizeof(msg), nullptr) < 0)
    {
        syslog(LOG_ERR, "ERROR: failed to read");
        return false;
    }
    return true;
}

bool conn_mq::Write(const std::string &msg) {
    if (mq_send(mq, (char *)(&msg), sizeof(msg), 0) < 0)
    {
        syslog(LOG_ERR, "ERROR: failed to write");
        return false;
    }
    return true;
}

conn_mq::~conn_mq() {
    mq_close(mq);
}


