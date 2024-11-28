#pragma once

#include <string>
#include <sys/syslog.h>
#include <semaphore.h>
#include <fcntl.h>
#include "../config.hpp"
#include <csignal>
#include <vector>

using namespace config;

template <class T>
class ClientConnection{
    T fromClientToHost;
    T fromHostToClient;

    int host_pid{}, pid{};
    std::string client_name;
public:
    ClientConnection(int host_pid, int pid) : host_pid(host_pid), pid(pid){
        sem_t* semaphore = sem_open((create_path(host_pid, pid) + "_init").c_str(), O_CREAT, 0777, 0);
        if (semaphore == SEM_FAILED)
            throw std::runtime_error("Error to create a semaphore!");

        std::cerr<<"local sem path"<<std::endl;
        client_name = "Client" + std::to_string(pid);
        fromClientToHost = T(pid, host_pid, true);
        fromHostToClient = T(host_pid, pid, true);
        // освобождаем семафор
        sem_post(semaphore);
        std::cerr<<"post local sem"<<std::endl;
    }

    ClientConnection() = default;
    ClientConnection(const ClientConnection&) = default;
    ClientConnection& operator = (const ClientConnection&) = default;
    ClientConnection(ClientConnection &&) = default;
    ClientConnection &operator=(ClientConnection &&) = default;


    void read_client_move(std::string & string) {
        fromClientToHost.Read(string);
    }

    void send_client_status(const bool client_status) {
        std::string str_status = client_status ? "1" : "0";
        fromHostToClient.Write(str_status);
        kill(pid, SIGUSR1);
    }

    void send_game_status(const bool game_status) {
        std::cerr<<"send_game_status"<<std::endl;
        std::cerr<<"client_pid " << pid<<std::endl;
        std::string str_status = game_status ? "1" : "0";
        std::cerr<<"game status "<<str_status<<std::endl;
        fromHostToClient.Write(str_status);
        kill(pid, SIGUSR2);
    }

    std::string getName() {
        return client_name;
    };

    ~ClientConnection() {
        syslog(LOG_INFO, "Close connection with client %s", client_name.c_str());
        std::cerr<<"~ClientConnection"<<std::endl;
    }
};
