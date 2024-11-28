#ifdef FIFO_Connection
#include "../conn_templates/fifo_temp.hpp"
#elif defined(MQ_Connection)
#include "../conn_templates/mq_temp.hpp"
#elif defined(SEG_Connection)
#include "../conn_templates/seg_temp.hpp"
#endif
#include "host.hpp"
#include "HostWindow.hpp"
#include "../config.hpp"


using namespace config;
namespace{
    Host<ConnType>& host = Host<ConnType>::getInstance(host_pid_path);
    HostWindow *window_ptr;
}

void host_signal_handler(const int sig, siginfo_t *info, void *context)
{
    std::cerr<<"host_signal_handler"<<std::endl;
    if (!window_ptr)
        return;
    if (!host.connects.contains(info->si_pid))
    {
        host.connects.emplace(info->si_pid, ClientConnection<ConnType>{getpid(), info->si_pid});
        if (host.connects.size() == 1) {
            window_ptr -> setHostNumber(host.wolf_number);
        }
        window_ptr->addClient(QString::fromStdString(host.connects[info->si_pid].getName()));
        return;
    }
    std::string client_move = " ";

    switch (sig)
    {
        case SIGUSR1: {
            host.connects[info->si_pid].read_client_move(client_move);
            syslog(LOG_NOTICE, "client's pid = %d move: %s", info->si_pid, client_move.c_str());

            const bool client_status = host.is_goatling_alife(info->si_pid, client_move);
            std::cerr<<"client's pid = %d status: "<<client_status<<std::endl;
            host.connects[info->si_pid].send_client_status(client_status);

            window_ptr -> updateClientStatus(QString::fromStdString(host.connects[info->si_pid].getName()), client_status);
            window_ptr -> updateClientNumber(QString::fromStdString(host.connects[info->si_pid].getName()), QString::fromStdString(client_move));
            client_move.clear();
            if (host.check_if_it_end_of_turn()) {
                window_ptr -> setHostNumber(host.wolf_number);
                window_ptr -> updateDeathCount(host.number_varies_when_everyone_dead);
            };
            break;
        }
        case SIGUSR2: {
            syslog(LOG_NOTICE, "Client %d dropped out of the game", info->si_pid);
            host.delete_client(info->si_pid);
            window_ptr -> deleteClient(QString::fromStdString(host.connects[info->si_pid].getName()));
            break;
        }
        default: {
            break;
        }
    }
}


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    HostWindow window;
    window_ptr = &window;

    window.show();

    return app.exec();
}