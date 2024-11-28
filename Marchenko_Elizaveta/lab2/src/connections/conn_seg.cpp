#include "conn_seg.hpp"
#include <sys/shm.h>
#include <fcntl.h>
#include <cstring>
#include <qguiapplication_platform.h>
#include <sys/syslog.h>

#include "../config.hpp"

using namespace config;

conn_seg::conn_seg(const int fromPid, const int toPid, const bool isHost) {
    _isHost = isHost;

    name = "/" + create_path(toPid, fromPid) + "seg";

    if (isHost) {
        _shmid = shmget(toPid, 1024, IPC_CREAT | O_EXCL | 0666);
    }
    else {
        _shmid = shmget(toPid, 1024, 0666);
    }

    if (_shmid == -1) {
        syslog(LOG_ERR, "Could not get shared memory segment");
        throw "Could not get shared memory segment";
    }


    _segptr = shmat(_shmid, nullptr, 0);
    if (_segptr == reinterpret_cast<void *>(-1)) {
        if (_isHost)
            shmctl(_shmid, IPC_RMID, 0);
        syslog(LOG_ERR, "Could not attach to shared memory segment");
        throw "Could not attach to shared memory segment";
    }
    std::cerr<<"Create " << name << std::endl;
}

bool conn_seg::Read(std::string& msg) {
    if (msg.size() > 1024)
        return false;

    // Создаем массив символов для копирования
    char buffer[1024];  // Размер массива должен быть достаточно большим, чтобы вместить сообщение

    // Копируем данные из message в buffer с помощью memcpy
    std::memcpy(buffer, _segptr, 1024);

    // Теперь копируем массив в std::string
    msg = std::string(buffer);

    return true;
}

bool conn_seg::Write(const std::string& msg) {
    if (msg.size() > 1024)
        return false;

    memcpy(_segptr, &msg, sizeof(msg));
    return true;
}

conn_seg::~conn_seg() {
    shmdt(_segptr);
    if (_isHost)
        shmctl(_shmid, IPC_RMID, nullptr);
    //std::cerr<<"Destroy " << name << std::endl;
}