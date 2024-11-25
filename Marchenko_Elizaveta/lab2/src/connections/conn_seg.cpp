#include "conn_seg.hpp"
#include <sys/shm.h>
#include <fcntl.h>
#include <cstring>
#include "../config.hpp"

using namespace config;

conn_seg::conn_seg(int fromPid, int toPid, bool isHost) {
    _isHost = isHost;

    int f = _isHost ? IPC_CREAT | O_EXCL | 0666 : 0666;

    std::string name = create_path(toPid, fromPid) + "seg";
    key_t key = ftok(name.c_str(), f);
    _shmid = shmget(key, 1024, f);
    if (_shmid == -1)
        throw "segment creation error";

    _segptr = shmat(_shmid, 0, 0);
    if (_segptr == (void*)-1) {
        if (_isHost)
            shmctl(_shmid, IPC_RMID, 0);
        throw "segment attachment error";
    }
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
    //memcpy(&msg, _segptr, sizeof(msg));
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
        shmctl(_shmid, IPC_RMID, 0);
}