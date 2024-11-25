#include "conn_mq.hpp"
#include <sys/syslog.h>

#include <sys/un.h>
#include "../config.hpp"
using namespace config;

conn_mq::conn_mq(int fromPid, int toPid, bool isHost) {
    name = "/" + create_path(fromPid, toPid) + "mq";
    if (isHost)
    {
        std::cerr << "Creating mq connection " << name << std::endl;
        mq_attr attr;
        attr.mq_maxmsg = 10;
        attr.mq_msgsize = 1024;
        attr.mq_curmsgs = 0;
        attr.mq_flags = 0;
        mq = mq_open(name.c_str(), O_CREAT | O_RDWR | O_NONBLOCK, 0777, &attr);
    }
    else
    {
        std::cerr<<"Opening mq connection " << name << std::endl;
        mq = mq_open(name.c_str(), O_RDWR | O_NONBLOCK, 0777);
    }

    if (mq < 0)
    {
        syslog(LOG_ERR, "ERROR: failed to create");
        throw std::runtime_error("failed to create MQ");
    }
}

bool conn_mq::Read(std::string &msg) {
    std::cerr<<"reading from mq "<<name<<std::endl;
    char buffer[1024];
    memset(buffer, 0, sizeof(buffer));

    mq = mq_open(name.c_str(), O_RDWR | O_NONBLOCK);
    if (mq < 0)
    {
        syslog(LOG_ERR, "ERROR: failed to create");
        throw std::runtime_error("failed to create MQ");
    }

    const ssize_t bytesReceived = mq_receive(mq, buffer, sizeof(buffer), nullptr);

    if (bytesReceived == -1)
    {
        return false;
    }

    msg.assign(buffer, bytesReceived);
    std::cerr << "Message received: " << msg << std::endl;
    return true;
}

bool conn_mq::Write(const std::string &msg) {
    std::cerr<<"sending to mq "<<name<<std::endl;
    std::cerr<<msg<<std::endl;
    mq = mq_open(name.c_str(), O_RDWR | O_NONBLOCK);
    if (mq < 0)
    {
        syslog(LOG_ERR, "ERROR: failed to create");
        throw std::runtime_error("failed to create MQ");
    }
    if (mq_send(mq, msg.c_str(), msg.size(), 0) < 0)
    {
        return false;
    }

    std::cerr << "Message sent: " << msg << std::endl;

    return true;
}

conn_mq::~conn_mq() {
    mq_close(mq);
}


