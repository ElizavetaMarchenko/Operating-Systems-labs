#pragma once

#include <string>
#include <sys/syslog.h>
#include <semaphore.h>
#include <fcntl.h>
#include "../config.hpp"
using namespace config;

template <class T>
class ClientConnection{
    T fromHostToClient;
    T fromClientToHost;
    int host_pid{}, pid{};
    std::string client_name;
public:
    ClientConnection(int host_pid, int pid) : host_pid(host_pid), pid(pid){
        // пляски с семафором
        sem_t* semaphore = sem_open((create_path(host_pid, pid) + "_init").c_str(), O_CREAT, 0777, 0);
        if (semaphore == SEM_FAILED)
            throw std::runtime_error("Error to create a semaphore!");

        std::cerr<<"local sem path"<<std::endl;
        client_name = "Client" + std::to_string(pid);
        fromClientToHost = T(host_pid, pid, true);
        fromHostToClient = T(pid, host_pid, true);
        // освобождаем семафор
        sem_post(semaphore);
        std::cerr<<"post local sem"<<std::endl;
    }
    ClientConnection() = default;

    void read_client_move(std::string & string) {
        fromClientToHost.Read(string);
    }

    void send_client_status(const bool client_status) {
        std::string str_status = client_status ? "1" : "0";
        fromHostToClient.Write(str_status);
        //сигнал
    }

    void send_game_status(const bool game_status) {
        std::string str_status = game_status ? "1" : "0";
        fromHostToClient.Write(str_status);
        //сигнал
    }

    std::string getName() {
        return client_name;
    };

    ~ClientConnection() {
        syslog(LOG_INFO, "Close connection with client %s", client_name.c_str());
    }
};
