#pragma once
/* MIT License
 *
 * Copyright (c) [year] [fullname]
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */

#include <queue>
#include <thread>
#include <atomic>
#include <type_traits>
#include <mutex>
#include <condition_variable>

namespace jstd
{
    template <typename ForwardType>
    class conveyor
    {
        static_assert(std::is_move_constructible<ForwardType>::value,
                      "The template parameter is not move constructable. "
                      "If this type cannot be made move constructable use std::unique_ptr<T>.");

    public:
        using ProcessorFunction = std::function<void(ForwardType&&)>;

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

        void push(ForwardType&& forwardValue)
        {
            std::unique_lock<std::mutex> lock(_lock);

            _queue.push(std::move(forwardValue));

            lock.unlock();
            _cv.notify_one();
        }

        template <typename T = ForwardType>
        typename std::enable_if<std::is_copy_constructible<T>::value >::type
        push(const T& forwardValue)
        {
            std::unique_lock<std::mutex> lock(_lock);

            _queue.push(forwardValue);

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
        std::queue<ForwardType> _queue;
        std::mutex _lock;
        std::condition_variable _cv;

        std::atomic_bool _shouldFinish { false };

        ProcessorFunction _processor;
        std::thread _thread;
    };
}