#include <iostream>
#include <string.h>
#include <functional>

#include "logger.h"

namespace Threads
{
bool SignalToThread::get_needToReleaseState()
{
    mutex_needToRelease.lock();
    if(needToReleaseState)
    {
        mutex_needToRelease.unlock();
        return true;
    }
    else
    {
        mutex_needToRelease.unlock();
        return false;
    }
}
void SignalToThread::set_needToReleaseState(const bool new_needToReleaseState)
{
    mutex_needToRelease.lock();
    needToReleaseState = new_needToReleaseState;
    mutex_needToRelease.unlock();
}

void Thread::start()
{
    thread = new std::thread(std::bind(&Thread::run, this));
    signal = new SignalToThread;
}
void Thread::signalToRelease()
{
    signal->set_needToReleaseState(true);
}
void Thread::release()
{
    thread->join();
    delete thread;
    delete signal;
}
void Thread::kill()
{
    thread->detach();
    thread->~thread();  // осталось поймать исключение внутри потока!)))
    // решение тут
    // https://www.bo-yang.net/2017/11/19/cpp-kill-detached-thread
    // в любом случае берём pthread.native_handle() и вызываем:
    // а) pthread_cancel для POSIX систем
    // б) другую хуйню для других систем
    //if(thread->joinable())
    //    std::terminate();
    //delete thread;
}

void Logger::send(const Message newMessage)
{
    mutex_queue.lock();
    messagesQueue.push(newMessage);
    cv_queue.notify_one();
    mutex_queue.unlock();
}
void Logger::sendBuffer(std::stringstream &buffer)
{
    Message m;
    m.type = Message::Type::stringstream;
    buffer.seekg(0, std::ios::end);
    int size = buffer.tellg();
    buffer.seekg(0, std::ios::beg);
    m.size = size + 1;  // + символ "\0"
    m.data = new char[m.size];
    buffer.read(m.data, size);
    m.data[m.size - 1] = 0;
    send(m);
    buffer.str("");
}
void Logger::run()
{
    for(;;)
    {
        // ожидание сообщения
        std::unique_lock<std::mutex> locker_queue(mutex_queue);
        for(;;)
        {
            if(!messagesQueue.empty())
                break;
            if(cv_queue.wait_for(locker_queue, std::chrono::milliseconds(1000)) == std::cv_status::timeout)
            {
                //std::cout << "[logger]listen..." << std::endl;
                if(signal->get_needToReleaseState() == true)
                {
                    goto exit;
                }
            }
        }
        for(;;)
        {
            if(messagesQueue.empty())
                break;
            Message &m = messagesQueue.front();
            locker_queue.unlock();
            // обработка
            workOnReceivedMessage(m);
            // обработка
            locker_queue.lock();
            messagesQueue.pop();
        }
        locker_queue.unlock();
    }
exit:;
}
}   //namespace Threads
