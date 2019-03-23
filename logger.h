#ifndef LOGGER_H
#define LOGGER_H

#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include "logger_base.h"

namespace Threads
{
// сигнал потоку
class SignalToThread: public SignalToThread_base
{
public:
    virtual bool get_needToReleaseState() override;
    virtual void set_needToReleaseState(const bool new_needToReleaseState) override;
    virtual ~SignalToThread(){};
protected:
    std::mutex mutex_needToRelease;
    bool needToReleaseState = false;
};
// поток
class Thread
{
public:
    // функция, запускающаяся в отдельном потоке
    virtual void run() = 0;
    // запуск
    void start();
    // послание сигнала о необходимости завершения
    void signalToRelease();
    // освобождение
    void release();
    // освободить принудительно (ф-я для даунов)
    // ## не доделанная ф-я , программа падает после такого завершения
    void kill();
    virtual ~Thread(){};
protected:
    SignalToThread *signal; // сигнал, с помощью которого можно сообщать потому о необходимости его завершения
private:
    std::thread *thread;
};
// логгер, который принимает и обрабатывает сообщения
// на signalToRelease среагирует только если возникнет интервал 1000 милисекунд, в течение которого придёт 0 сообщений
// требуется реализация workOnReceivedMessage
class Logger: public Logger_base, public Thread
{
public:
    virtual void send(const Message newMessage) override;
    virtual void sendBuffer(std::stringstream &buffer) override;
    virtual ~Logger(){};
protected:
    // основная функция процесса логирования
    virtual void run() override;
    std::queue<Message> messagesQueue;  // очередь сообщений
    std::mutex mutex_queue;             // для синхронного доступа к очереди
    std::condition_variable cv_queue;   // для отправки сигнала от потока логгеру, о том, что послано сообщение
};
}   //namespace Threads
#endif // LOGGER_H
