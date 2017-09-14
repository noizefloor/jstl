#pragma once

#include <functional>
#include <mutex>
#include <queue>
#include <future>

namespace jstd
{
    namespace internal
    {
        template<typename ForwardT, typename ConsumerT = std::function<void(ForwardT&&)> >
        class conveyor_forwarder
        {
        public:
            explicit conveyor_forwarder(ConsumerT&& consumer)
                : _consumer(std::move(consumer))
                , _consumerHandle(std::async(std::launch::async, [this] { run(); }))
            {
            }

            template<typename T>
            void push(T&& forwardValue)
            {
                checkForError();

                std::unique_lock<std::mutex> lock(_mutex);

                _queue.push(std::forward<T>(forwardValue));

                lock.unlock();
                _cv.notify_one();
            }

            void finish()
            {
                _shouldFinish = true;
                _cv.notify_one();
                _consumerHandle.wait();
            }

            void checkForError() const
            {
                if (_hasError)
                    std::rethrow_exception(_error);
            }

        private:

            void run()
            {
                try
                {
                    while (true)
                    {
                        std::unique_lock<std::mutex> lock(_mutex);

                        if (_queue.empty() && !_shouldFinish)
                            _cv.wait(lock, [this]
                            {
                                return !_queue.empty() || _shouldFinish;
                            });

                        if (!_queue.empty())
                        {
                            auto value = std::forward<ForwardT>(_queue.front());
                            _queue.pop();

                            lock.unlock();

                            _consumer(std::forward<ForwardT>(value));
                        }
                        else if (_shouldFinish)
                            return;
                    }
                }
                catch (...)
                {
                    _error = std::current_exception();
                    _hasError = true;
                }
            }

        private:
            std::queue<ForwardT> _queue;
            std::mutex _mutex;
            std::condition_variable _cv;

            std::atomic_bool _shouldFinish { false };

            ConsumerT _consumer;
            std::future<void> _consumerHandle;

            std::exception_ptr _error;
            std::atomic_bool _hasError { false };
        };

    } // internal

} // jstd