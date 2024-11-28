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
#include <filesystem>
#include <memory>

#include "../config.hpp"
using namespace config;

conn_fifo::conn_fifo(int fromPid, int toPid, bool isHost) {
    _isHost = isHost;
    _fifoName = std::filesystem::current_path() / (create_path(fromPid, toPid) + "fifo");
    std::cerr << "Created fifo " << _fifoName << std::endl;

    if (_isHost)
        if (mkfifo(_fifoName.c_str(), 0777) == -1)
        {
            std::cerr << "Already created " << _fifoName << std::endl;
            // Если файл уже существует, игнорируем ошибку
            if (errno != EEXIST)
            {
                std::cerr << std::strerror(errno) << std::endl;
                throw std::runtime_error("Cannot create FIFO channel");
            }
        }

    _fileDescr = open(_fifoName.c_str(), O_RDWR);
    if (_fileDescr == -1) {
        std::cerr << _fifoName << " " << std::strerror(errno) << std::endl;
        if (_isHost)
            unlink(_fifoName.c_str());
        throw "fifo openning error";
    }
    std::cerr<<"fifo opened"<<std::endl;
}

bool conn_fifo::Read(std::string& msg) {
    char buffer[1024];
    memset(buffer, '\0', 1024);

    // Открытие FIFO канала для чтения
    _fileDescr = open(_fifoName.c_str(), O_RDWR);
    if (_fileDescr == -1)
    {
        throw std::runtime_error("Failed to open FIFO channel");
    }

    const ssize_t bytes_read = read(_fileDescr, buffer, 1024 - 1);
    if (bytes_read == -1)
    {
        return false;
    }

    msg.assign(buffer, bytes_read);
    return true;
}

bool conn_fifo::Write(const std::string& msg) {
    // Открытие FIFO канала для записи
    _fileDescr = open(_fifoName.c_str(), O_RDWR);
    if (_fileDescr == -1)
    {
        throw std::runtime_error("Failed to open FIFO channel");
    }

    ssize_t bytesWritten = write(_fileDescr, msg.c_str(), msg.size());
    if (bytesWritten == -1)
    {
        return false;
    }
    return true;
}

conn_fifo::~conn_fifo() {
    close(_fileDescr);
    if (_isHost)
        unlink(_fifoName.c_str());
    std::cerr << "Closing fifo " << _fifoName << std::endl;
}