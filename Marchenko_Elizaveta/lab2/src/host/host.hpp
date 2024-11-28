#pragma once
#include "ClientConnection.hpp"
#include <map>
#include <fstream>
#include <csignal>
#include <chrono>
#include <unordered_map>
#include <filesystem>
#include <iostream>
#include <csignal>
#include <sys/stat.h>
#include <sys/types.h>
#include <syslog.h>
#include <cstring>
#include <stdexcept>
#include <semaphore>
#include <memory>

void host_signal_handler(int, siginfo_t *, void *);

template<class T>
class Host {

    int pid;
    std::string pid_path;
    std::unordered_map<int, ClientConnection<T>> connects;
    struct sigaction signal_handler;
    sem_t* game_sem;


    //Механики игры
    std::unordered_map<int, bool> status; // живые или мёртвые
    std::map<int, int> last_move;
    unsigned int number_varies_when_everyone_dead = 0;
    int wolf_number;
    unsigned int number_of_made_move_clients;
    unsigned int number_of_dead_clients;

    friend void host_signal_handler(int, siginfo_t *, void *);

    Host(const std::string &pid_path): pid_path(pid_path) {
        openlog("host.log", LOG_PID, LOG_USER);
        std::ofstream file(pid_path);
        if (!file)
            throw std::runtime_error("No config file: " + std::string(std::filesystem::absolute(pid_path)));
        pid = getpid();
        file << pid;
        std::cerr << pid << std::endl;
        file.close();
        syslog(LOG_NOTICE, "Host pid: %d", pid);
        signal_handler.sa_sigaction = host_signal_handler;
        signal_handler.sa_flags = SA_SIGINFO;

        if (sigaction(SIGUSR1, &signal_handler, nullptr) == -1)
        {
            syslog(LOG_ERR, "Host: sigaction failed");
            throw std::runtime_error("Failed to register signal handler");
        }

        if (sigaction(SIGUSR2, &signal_handler, nullptr) == -1)
        {
            syslog(LOG_ERR, "Host: sigaction failed");
            throw std::runtime_error("Failed to register signal handler");
        }

        game_sem = sem_open((std::to_string(pid) + "_game").c_str(), O_CREAT, 0777, 1);
        std::cerr<<"open game sem " << (std::to_string(pid) + "_game") << std::endl;
        choose_wolf_number();
    }

    void delete_client(int pid) {
        // блок семафора
        block_sem();
        last_move.erase(pid);
        status.erase(pid);
        if (status.empty()) {
            syslog(LOG_INFO, "No players. End game.");
            exit(0);
        }
        //открыть семафор
        sem_post(game_sem);
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
        std::cerr<<"block_sem"<<std::endl;
    }



    bool check_if_it_end_of_turn() {
        std::cerr<<"check_if_it_end_of_turn"<<std::endl;
        std::cerr<<number_of_made_move_clients<<' '<<status.size()<<std::endl;
        if (number_of_made_move_clients == status.size()) {
            // Это конец хода
            // Блокируем семафор
            block_sem();

            syslog(LOG_INFO, "End of turn");
            if (number_of_dead_clients == status.size()) {
                number_varies_when_everyone_dead ++;
                syslog(LOG_INFO, "Number of dead clients %d", number_of_dead_clients);
            }
            else {
                number_varies_when_everyone_dead = 0;
            }

            number_of_dead_clients = 0;

            bool game_continue = true;
            if (number_varies_when_everyone_dead == 2) {
                syslog(LOG_INFO, "End of game");
                game_continue = false;
            }
            number_of_made_move_clients = 0;

            std::cerr<<"game_status "<<game_continue<<std::endl;
            for (auto [client_pid, _]: last_move) {
                connects[client_pid].send_game_status(game_continue);
            }

            if (game_continue) {
                choose_wolf_number();
                syslog(LOG_INFO, "NEW ROUND");
            }
            else {
                exit(0); //???
            }

            // Открываем семафор
            sem_post(game_sem);
            std::cerr<<"post sem"<<std::endl;
            return true;
        }
        return false;
    };



    void choose_wolf_number() {
        wolf_number = rand() % 100 + 1;
        syslog(LOG_INFO, "Wolf number: %d", wolf_number);
    }

    bool is_goatling_alife(int goatling_pid, const std::string &client_move) {
        number_of_made_move_clients++;
        int goatling_number;
        try {
            goatling_number = std::stoi(client_move);
        }
        catch (...) {
            goatling_number = -1000;
        }
        last_move[goatling_pid] = goatling_number;
        if (std::abs(goatling_number - wolf_number) < 70 / connects.size() && status[goatling_pid]){
            status[goatling_pid] = true;
            return true;
        }
        if (std::abs(goatling_number - wolf_number) < 20 / connects.size() && !status[goatling_pid]){
            status[goatling_pid] = true;
            return true;
        }
        status[goatling_pid] = false;
        number_of_dead_clients++;
        return false;
    }

public:
    static Host &getInstance(const std::string &pid_path)
    {
        static Host instance(pid_path);
        return instance;
    }

    ~Host()
    {
        remove(pid_path.c_str());
        closelog();
        unlink((std::to_string(pid) + "_game").c_str());
    }
};



