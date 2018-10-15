#pragma once

// MIT License
//
// Copyright (c) 2017 Jan Schwers
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include <queue>
#include <future>
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
        explicit conveyor(const ProcessorFunction& processor)
            : processor_(processor)
            , processorHandle_(std::async(std::launch::async, [this] { run(); }))
        {
        }

        explicit conveyor(ProcessorFunction&& processor)
            : processor_(std::move(processor))
            , processorHandle_(std::async(std::launch::async, [this] { run(); }))
        {
        }

        ~conveyor()
        {
            {
                std::lock_guard<std::mutex> lock(guard_);
                shouldFinish_ = true;
            }

            cv_.notify_one();

            processorHandle_.wait();
        }

        void push(ForwardType&& forwardValue)
        {
            push_internal(std::move(forwardValue));
        }

        template <typename T = ForwardType>
        void push(const T& forwardValue,
                  typename std::enable_if<std::is_copy_constructible<T>::value>::type* = 0 )
        {
            push_internal(forwardValue);
        }

    private:

        template <typename T>
        void push_internal(T&& forwardValue)
        {
            std::unique_lock<std::mutex> lock(guard_);

            queue_.push(std::forward<T>(forwardValue));

            lock.unlock();
            cv_.notify_one();
        }

        void run()
        {
            while(true)
            {
                std::unique_lock<std::mutex> lock(guard_);

                if (queue_.empty() && !shouldFinish_)
                    cv_.wait(lock, [this] { return !queue_.empty() || shouldFinish_; });

                if (!queue_.empty())
                {
                    auto value = std::forward<ForwardType>(queue_.front());
                    queue_.pop();

                    lock.unlock();

                    processor_(std::forward<ForwardType>(value));
                }
                else if (shouldFinish_)
                    return;
            }
        }

    private:
        std::queue<ForwardType> queue_;
        std::mutex guard_;
        std::condition_variable cv_;

        bool shouldFinish_ { false };

        ProcessorFunction processor_;
        std::future<void> processorHandle_;
    };
}