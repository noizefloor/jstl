#pragma once

#include <queue>
#include <thread>
#include <atomic>

namespace jstd
{
    template <typename T>
    class conveyor
    {
    public:
        using ProcessorFunction = std::function<void(T&&)>;

    public:
        conveyor(const ProcessorFunction& processor)
            : _processor(processor)
            , _thread([this] { run(); })
        {
        }

        conveyor(ProcessorFunction&& processor)
            : _processor(std::move(processor))
            , _thread([this] { run(); })
        {
        }

        ~conveyor()
        {
            _shouldFinish = true;

            _cv.notify_one();

            if (_thread.joinable())
                _thread.join();
        }

        void push(T&& forwardValue)
        {
            std::unique_lock<std::mutex> lock(_lock);

            _queue.push(std::move(forwardValue));

            lock.unlock();
            _cv.notify_one();
        }

    private:

        void run()
        {
            while(true)
            {
                std::unique_lock<std::mutex> lock(_lock);

                if (_queue.empty() && !_shouldFinish)
                    _cv.wait(lock, [this] { return !_queue.empty() || _shouldFinish; });

                if (!_queue.empty())
                {
                    auto value = std::move(_queue.front());
                    _queue.pop();

                    lock.unlock();

                    _processor(std::move(value));
                }
                else if (_shouldFinish)
                    return;
            }
        }

    private:
        ProcessorFunction _processor;
        std::thread _thread;

        std::queue<T> _queue;
        std::mutex _lock;
        std::condition_variable _cv;

        std::atomic_bool _shouldFinish { false };

    };
}