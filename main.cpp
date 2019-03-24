#include <iostream>
#include "logger.h"

/*
   для добавления поддержки ногого типа сообщения логгером сделать следующее:
   1) добавить новый тип сообщения в Message::Type
   2) расширить интерфейс Logger_base (можно не менять и посылать сообщение функцией send, заворачивая руками)
   3) добавить обработку нового типа сообщения в реализацию workOnReceivedMessage

   to do, баги:
   - время раздупления = завершения логгера после послания сигнала о необходимости завершения = 1 секунда - зашито константой внутрь логгера
   - kill потока руинит работу программы
*/\

const int threadsNumber = 30;   // система может ограничивать большое количество потоков
const int messagesNumber = 20;
const int iterationsNumber = 10;

// решатель (для него требуется подключить только "logger_base.h" для ускорения компиляции)
class Solver: public Threads::Logging
{
public:
    // решение задачи
    void solve(Threads::SignalToThread_base *set_signal, Threads::Logger_base *set_logger, int id)
    {
        initLogging(set_signal, set_logger);
        outStream << "[solver" << id << "]\tstarted" << std::endl;
        logger->sendBuffer(outStream);
        // процесс
        for(int i = 0; i <= 10; i++)
        {
            if(signal->get_needToReleaseState())
            {
                outStream << "[solver" << id << "]\taborted :( " << std::endl;
                logger->sendBuffer(outStream);
                return;
            }
            outStream << "[solver" << id << "] data№ " << i << std::endl;
            logger->sendBuffer(outStream);
        }
        outStream << "[solver" << id << "]\tfinished" << std::endl;
        logger->sendBuffer(outStream);
    }
};

// логгер для решателя - реализация обработки сообщений
class SolverLogger: public Threads::Logger
{
public:
    virtual void workOnReceivedMessage(Threads::Message newMessage) override
    {
        if(newMessage.type == Threads::Message::Type::stringstream)
            std::cout << "message: " << std::string(newMessage.data);
    }
};

// поток решателя
// решатель должен иметь функцию (например, solve), принимающую логгер, signal, и, если нужно, другие данные(например, id),
// и вызывать в начале выполнения initLogging,
// тогда можно безболезненно пользоваться логгером, как показано в ф-и solve
// (сигнал создаётся в конструкторе Threads::Thread, поэтому для инициализации он не нужен)
struct SolverThread: public Threads::Thread
{
    Solver s;
    Threads::Logger_base *logger;
    int id; // индекс потока - для теста, в реальности - любые входные данные
    void init(Threads::Logger_base *set_logger, const int set_id)
    {
        logger = set_logger;
        id = set_id;
    }
    virtual void run() override
    {
        s.solve(signal, logger, id);    // сигнал создаётся в конструкторе Threads::Thread для каждого потока
    }
};

int main()
{
    SolverLogger logger;
    SolverThread solver[threadsNumber];
    for(int k = 0; k < iterationsNumber; k++)
    {
        std::cout << std::endl << "Iteration = " << k << std::endl;
        logger.start();
        for(int i = 0; i < threadsNumber; i++)
        {
            solver[i].init(&logger, i);
            solver[i].start();  // потоки равномерно присасываются к логгеру
        }
        //std::this_thread::sleep_for(std::chrono::milliseconds(1));
        for(int i = 0; i < threadsNumber; i++)
            solver[i].signalToRelease();    // просим потоки освободиться
        logger.signalToRelease();
        for(int i = 0; i < threadsNumber; i++)
            solver[i].release();  // ждём освобождения потоков
        logger.release();   // ждём освобождения логгера
        // нужна 1 сикунда полной тишины, чтобы логгер среагировал на signalToRelease
        // ## время раздупления - 1 секунда - намертво зашита в логгер
        //std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }   // повторяем итерации: повторно создаём логгер и потоки-спиногрызы
    printf("fix number 1\n");
    printf("fix number 2\n");
    return 0;
}
