#include "conn_fifo.hpp"
#include <thread>
#include <chrono>
#include <functional>
#include <filesystem>
#include <fstream>
#include <csignal>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <cstring>
#include <semaphore>
#include <memory>

#include "../config.hpp"
using namespace config;

conn_fifo::conn_fifo(int fromPid, int toPid, bool isHost) {
    _isHost = isHost;
    _fifoName = "/" + create_path(fromPid, toPid) + "fifo";

    if (_isHost)
        if (mkfifo(_fifoName.c_str(), 0666))
            throw "fifo creation error";

    _fileDescr = open(_fifoName.c_str(), O_RDWR);
    if (_fileDescr == -1) {
        if (_isHost)
            unlink(_fifoName.c_str());
        throw "fifo openning error";
    }
}

bool conn_fifo::Read(std::string& msg) {
    std::cerr<<"Reading from FIFO"<<std::endl;
    if (read(_fileDescr, &msg, sizeof(msg)) < 0)
        return false;
    return true;
}

bool conn_fifo::Write(const std::string& msg) {
    std::cerr<<"Try to write msg: "<<msg<<std::endl;
    if (write(_fileDescr, &msg, sizeof(msg)) < 0)
        return false;
    return true;
}

conn_fifo::~conn_fifo() {
    close(_fileDescr);
    if (_isHost)
        unlink(_fifoName.c_str());
}