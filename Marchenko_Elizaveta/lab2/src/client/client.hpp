#pragma once
#include <fstream>
#include <csignal>
#include <chrono>
#include <filesystem>
#include <csignal>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <cstring>
#include <semaphore.h>
#include <memory>
#include "../config.hpp"
#include <fcntl.h>
using namespace config;
void client_signal_handler(int, siginfo_t *, void *);

template <typename T>
class Client {
    sem_t* global_semaphore;
    T fromClientToHost;
    T fromHostToClient;
    int host_pid, pid;
    bool status;
    struct sigaction signal_handler{};

    sem_t* game_sem;

    friend void client_signal_handler(int, siginfo_t *, void *);
    friend class ClientWindow;

    Client(const std::string& pid_path) {
        openlog("client", LOG_PID, LOG_USER);
        status = true;

        std::fstream pid_file(pid_path);
        if (!pid_file)
        {
            throw std::runtime_error(
                "Configuration file not found: " + std::filesystem::absolute(pid_path).string());
        }

        pid_file >> host_pid;
        pid_file.close();
        syslog(LOG_NOTICE, "Server PID: %d", host_pid);

        pid = getpid();
        std::cerr<<"Client PID: "<<pid<<std::endl;
        syslog(LOG_NOTICE, "Client PID: %d", pid);

        signal_handler.sa_sigaction = client_signal_handler;
        signal_handler.sa_flags = SA_SIGINFO;

        if (sigaction(SIGUSR1, &signal_handler, nullptr) == -1 ||
            sigaction(SIGUSR2, &signal_handler, nullptr) == -1)
        {
            throw std::runtime_error("Failed to register signal handlers.");
        }

        kill(host_pid, SIGUSR1);
        std::cerr<<"Host PID: "<<host_pid<<std::endl;
        std::cerr<<"Send meet"<<std::endl;

        sem_t* semaphore = sem_open(
            (create_path(host_pid, pid) + "_init").c_str(),
            O_CREAT, 0777, 0);
        std::cerr<<"local sem"<<std::endl;
        timespec timeout{};
        if (clock_gettime(CLOCK_REALTIME, &timeout) == -1)
        {
            throw std::runtime_error("Failed to retrieve current time.");
        }

        timeout.tv_sec += 5;

        int sem_wait_result;
        while ((sem_wait_result = sem_timedwait(semaphore, &timeout)) == -1 && errno == EINTR)
        {
            continue;
        }

        if (sem_wait_result == -1)
        {
            throw std::runtime_error("Semaphore wait timed out.");
        }

        fromClientToHost = T(pid, host_pid, false);
        fromHostToClient = T(host_pid, pid, false);
        // отсоединяем мелкий семафор
        sem_unlink((create_path(host_pid, pid) + "_init").c_str());
        std::cerr<<"unlink local sem"<<std::endl;

        game_sem = sem_open((std::to_string(host_pid) + "_game").c_str(), O_RDWR, 0777);
        std::cerr<<"open game sem " << (std::to_string(host_pid) + "_game") << std::endl;
        int sem_v;
        std::cerr<<"try to get value"<<std::endl;
        sem_getvalue(game_sem, &sem_v);
        std::cerr<<"sem_v: "<<sem_v<<std::endl;
    }

    void block_sem() const {
        timespec timeout{};
        if (clock_gettime(CLOCK_REALTIME, &timeout) == -1)
        {
            throw std::runtime_error("Failed to retrieve current time.");
        }

        timeout.tv_sec += 5;

        int sem_wait_result;
        while ((sem_wait_result = sem_timedwait(game_sem, &timeout)) == -1 && errno == EINTR)
        {
            continue;
        }

        if (sem_wait_result == -1)
        {
            throw std::runtime_error("Semaphore wait timed out.");
        }
        std::cerr<<"block sem"<<std::endl;
    }

    void send_move_to_host(const std::string& move){
          syslog(LOG_INFO, "Sending move to host. Move: %s.", move.c_str());
          std::cerr<<"Sending move to host. Move: "<<move<<std::endl;

          block_sem();
          fromClientToHost.Write(move);
          kill(host_pid, SIGUSR1);
    }

    void read_game_status(std::string& st){
          fromHostToClient.Read(st);
    }
    void read_life_status(std::string& st) {
        fromHostToClient.Read(st);
        if (static_cast<int>(st[0]) == 1) {
            status = true;
        }
        else {
            status = false;
        }
    }

    bool getStatus() const {
        return status;
    }

public:
    static Client &getInstance(const std::string &pid_path)
    {
        static Client instance(pid_path);
        return instance;
    }

    ~Client(){
        syslog(LOG_INFO, "Closing client");
        kill(host_pid, SIGUSR2);
        closelog();
    }

};




