#pragma once


#include <QMainWindow>
#include <QVBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QTimer>
#include <QMessageBox>

class ClientWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit ClientWindow(QWidget *parent = nullptr) : QMainWindow(parent), playerStatus("Жив")
    {
        // Настройка виджетов
        QWidget *centralWidget = new QWidget(this);
        QVBoxLayout *layout = new QVBoxLayout(centralWidget);

        statusLabel = new QLabel("Статус: " + playerStatus, this);
        chosenNumberLabel = new QLabel("Выбранное число: —", this);
        inputField = new QLineEdit(this);
        sendButton = new QPushButton("Отправить", this);
        sendButton->setEnabled(false); // Кнопка по умолчанию отключена

        layout->addWidget(statusLabel);
        layout->addWidget(chosenNumberLabel);
        layout->addWidget(inputField);
        layout->addWidget(sendButton);

        centralWidget->setLayout(layout);
        setCentralWidget(centralWidget);

        // Настройка таймера
        turnTimer = new QTimer(this);
        turnTimer->setInterval(3000); // 3 секунды

        // Подключение сигналов и слотов
        connect(sendButton, &QPushButton::clicked, this, &ClientWindow::onSendButtonClicked);
        connect(turnTimer, &QTimer::timeout, this, &ClientWindow::onTimeout);

        startNewTurn();
    }

    void onTimeout() const;           // Обработчик тайм-аута

    void startNewTurn() const {
        inputField->clear();
        inputField->setEnabled(true);
        sendButton->setEnabled(true);
        turnTimer->start();
    } // Начало нового хода

    void endGame() const {
        statusLabel->setText("Конец игры");
        inputField->setEnabled(false);
        sendButton->setEnabled(false);
        turnTimer->stop();
    }// Завершение игры

    void updateStatus(const bool status) {
        playerStatus = status ? "Жив" : "Мёртв";
        statusLabel->setText("Статус: " + playerStatus);
    }

    ~ClientWindow() override {
        QMessageBox::information(this, "Игра завершена", "Спасибо за игру!");
    }
    public slots:
            void onSendButtonClicked() const; // Обработчик нажатия кнопки "Отправить"
private:
    QLabel *statusLabel;        // Отображение статуса (жив/мёртв)
    QLabel *chosenNumberLabel;  // Отображение выбранного числа
    QLineEdit *inputField;      // Поле ввода числа
    QPushButton *sendButton;    // Кнопка "Отправить"
    QTimer *turnTimer;          // Таймер для завершения хода
    QString playerStatus;       // Статус игрока
};



