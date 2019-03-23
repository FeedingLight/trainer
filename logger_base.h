#ifndef LOGGER_BASE_H
#define LOGGER_BASE_H

#include <cstddef>
#include <sstream>

namespace Threads
{
// сигнал потоку
class SignalToThread_base
{
public:
    // получение значения сигнала (true/false)
    virtual bool get_needToReleaseState() = 0;
    // изменение значения сигнала
    virtual void set_needToReleaseState(const bool new_needToReleaseState) = 0;
    virtual ~SignalToThread_base(){};
};
// сообщение логгеру
struct Message
{
    enum class Type
    {
        stringstream = 0,
    };
    Type type;
    size_t size;
    char *data;
};
// логгер, который принимает и обрабатывает сообщения
class Logger_base
{
public:
    // отправка сообщения логгеру
    // сообщение будет находится в очереди, пока не обработается, поэтому лучше выделить память для копии данных в newMessage
    virtual void send(const Message newMessage) = 0;
    // заворачивание в объект Message текстового сообщения, сформированного в buffer,
    // отправка посредством send, и очистка buffer
    virtual void sendBuffer(std::stringstream &buffer) = 0;
    // функция-обработчик вызывается логгером для каждого полученного сообщения
    // посылается копия данных
    // освобождение памяти на стороне обработчика сообщений
    virtual void workOnReceivedMessage(Message newMessage) = 0;
    virtual ~Logger_base(){};
};
// содержит компоненты для логгирования
class Logging
{
public:
    void initLogging(Threads::SignalToThread_base *set_signal, Threads::Logger_base *set_logger)
    {
        logger = set_logger;
        signal = set_signal;
        outStream.str("");
    }
    Threads::Logger_base *logger;           // логгер
    Threads::SignalToThread_base *signal;   // для передачи сигналов решателю
    std::stringstream outStream;            // поток вывода текста, который отправляется логгеру
};
}   //namespace Threads

#endif // LOGGER_BASE_H
