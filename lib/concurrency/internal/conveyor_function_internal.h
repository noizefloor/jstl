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

#include <functional>
#include <mutex>
#include <queue>
#include <future>
#import "../../type_traits/function_traits.h"

namespace jstd
{
    namespace internal
    {
        template <typename T>
        using is_consumer = std::integral_constant
                <bool, has_void_return_type<T>::value && function_traits<T>::arity == 1 &&
                        std::is_rvalue_reference< typename function_traits<T>::template arg<0>::type >::value>;

        template <typename T, typename ParameterT>
        using is_producer = std::integral_constant
                <bool, has_void_return_type<T>::value && function_traits<T>::arity == 1 &&
                        std::is_same<ParameterT, typename function_traits<T>::template arg<0>::type>::value>;

        template <typename T>
        struct consumer_variable : public std::decay<typename function_traits<T>::template arg<0>::type>
        {};


        template<typename ForwardT, typename ConsumerT>
        class conveyor
        {
        public:
            explicit conveyor(ConsumerT&& consumer)
                : _consumer(std::forward<ConsumerT>(consumer))
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