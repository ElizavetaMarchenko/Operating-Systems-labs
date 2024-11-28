#pragma once
#include <QMainWindow>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QVBoxLayout>
#include <QMap>
#include <QString>
#include <QApplication>
#include <QMessageBox>
#include <unordered_map>
#include <unistd.h>
#include <qt6/QtCore/QString>


class HostWindow : public QMainWindow {
    Q_OBJECT

public:
    explicit HostWindow(QWidget *parent = nullptr) : QMainWindow(parent) {
        // Основной виджет
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        // Метка для отображения числа хоста
        hostNumberLabel = new QLabel("Число хоста (волка): 0", this);

        // Список клиентов
        clientsListWidget = new QListWidget(this);

        // Метка для количества ходов, в течение которых все козлята мертвы
        deadCountLabel = new QLabel("Ходов, пока все мертвы: 0", this);

        // Основной компоновщик
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);
        layout->addWidget(hostNumberLabel);
        layout->addWidget(clientsListWidget);
        layout->addWidget(deadCountLabel);
    }

    void addClient(const QString &name) {
        if (!clients.contains(name)) {
            clients[name] = {true, 0};
            updateClientList();
        }
    }

    void updateClientStatus(const QString &name, bool alive) {
        if (clients.contains(name)) {
            clients[name].first = alive;
            updateClientList();
        }
    }

    void updateClientNumber(const QString &name, const QString &number) {
        if (clients.contains(name)) {
            clients[name].second = number;
            updateClientList();
        }
    }

    void updateDeathCount(const unsigned int &count) {
        deadCountLabel->setText("Ходов, пока все мертвы: " + QString::number(count));
    }

    void deleteClient(const QString &name) {
        if (clients.contains(name)) {
            clients.erase(name);
            updateClientList();
        }
    }

    void setHostNumber(int number) {
        hostNumberLabel->setText(QString("Число хоста (волка): %1").arg(number));
    }
    ~HostWindow() override {
        QMessageBox::information(this, "Игра завершена", "Спасибо за игру!");
    }


private:
    QLabel *hostNumberLabel;
    QListWidget *clientsListWidget;
    QLabel *deadCountLabel;
    std::unordered_map<QString, std::pair<bool, QString>> clients;
    void updateClientList() {
        clientsListWidget->clear();
        for (auto it : clients) {
            QString status = it.second.first ? "Жив" : "Мертв";
            clientsListWidget->addItem(QString("%1: %2, Число: %3")
                                           .arg(it.first)
                                           .arg(status)
                                           .arg(it.second.second));
        }
    }
};


