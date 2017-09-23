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

#include "internal/conveyor_function_internal.h"

#include <type_traits>
#include <memory>

namespace jstd
{
    namespace conveyor_internal
    {
        class conveyor_proxy
        {
        public:
            virtual ~conveyor_proxy() = default;

            virtual void finish() = 0;
            virtual void checkForError() const = 0;
        };

        template<typename T, typename Callable>
        class conveyor : public conveyor_proxy
        {
            class conveyor_forwarder_impl : public jstd::conveyor_forwarder<T>
            {
            public:
                explicit conveyor_forwarder_impl(conveyor<T, Callable>& forwarder)
                        : _forwarder(forwarder)
                {
                }

                virtual ~conveyor_forwarder_impl() = default;

                void push(T&& forwardValue) override
                {
                    _forwarder.push(std::move(forwardValue));
                }

                void push(const T& forwardValue) override
                {
                    static_assert(std::is_copy_constructible<T>::value,
                                  "Pushing a non copyable type by reference is not supported. "
                                  "Consider using std::move.");
                    _forwarder.push(forwardValue);
                }

            private:
                conveyor<T, Callable>& _forwarder;
            };

        public:
            explicit conveyor(Callable&& consumer)
                    : _consumer(std::forward<Callable>(consumer))
                      , _forwarder(*this)
                      , _consumerHandle(std::async(std::launch::async, [this] { run(); }))
            {
            }

            virtual ~conveyor() = default;

            template<typename ForwardType>
            void push(ForwardType&& forwardValue)
            {
                checkForErrorInternal();

                std::unique_lock<std::mutex> lock(_mutex);

                _queue.push(std::forward<ForwardType>(forwardValue));

                lock.unlock();
                _cv.notify_one();
            }

            void finish() override
            {
                _shouldFinish = true;
                _cv.notify_one();

                if (_consumerHandle.valid())
                    _consumerHandle.wait();

                if (_proxy)
                    _proxy->finish();
            }

            void checkForError() const override
            {
                if (_proxy)
                    _proxy->checkForError();

                checkForErrorInternal();
            }

            void setConveyorProxy(std::unique_ptr<conveyor_proxy>&& proxy)
            {
                _proxy = std::move(proxy);
            }

            conveyor_forwarder_impl& getForwarder()
            {
                return _forwarder;
            }

        private:

            void checkForErrorInternal() const
            {
                if (_hasError)
                    rethrow_exception(_error);
            }

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
                            auto value = std::move(_queue.front());
                            _queue.pop();

                            lock.unlock();

                            _consumer(std::move(value));
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
            std::unique_ptr<conveyor_proxy> _proxy;
            Callable _consumer;

            std::queue<T> _queue;
            std::mutex _mutex;
            std::condition_variable _cv;

            std::atomic_bool _shouldFinish { false };

            conveyor_forwarder_impl _forwarder;
            std::future<void> _consumerHandle;

            std::exception_ptr _error;
            std::atomic_bool _hasError { false };
        };

        template <typename T,
                  typename SourceType = typename callable_type<T>::source_type,
                  typename ConveyorType = conveyor<SourceType, T> >
        std::unique_ptr<ConveyorType> make_conveyor(T&& consumer)
        {
            return std::make_unique<ConveyorType>(std::forward<T>(consumer));
        };

        template <typename T, typename... Args,
                  typename SourceType = typename callable_type<T>::source_type,
                typename std::enable_if<callable_type<T>::callable == CallableType::converter, int>::type = 0>
        auto make_conveyor(T&& converter, Args&&... args)
        {
            auto&& conveyor = make_conveyor(std::forward<Args>(args)...);
            auto& forwarder = conveyor->getForwarder();

            auto&& consumer = [&forwarder, cv = std::forward<T>(converter)](SourceType&& value)
            {
                cv(std::move(value), forwarder);
            };

            auto&& resultConveyor = make_conveyor(std::move(consumer));
            resultConveyor->setConveyorProxy(std::move(conveyor));

            return std::move(resultConveyor);
        };
    }


    template <typename FirstCallable, typename SecondCallable, typename... CallableRest>
    void conveyor_function(FirstCallable&& producer, SecondCallable&& converterOrConsumer, CallableRest&&... args)
    {
        conveyor_internal::assert_signature<FirstCallable, SecondCallable, CallableRest...>();

        auto&& conveyor =
                conveyor_internal::make_conveyor(std::forward<SecondCallable>(converterOrConsumer),
                                                 std::forward<CallableRest>(args)...);
        try
        {
            producer(conveyor->getForwarder());
        }
        catch (...)
        {
            conveyor->finish();
            throw;
        }

        conveyor->finish();
        conveyor->checkForError();
    };


} // jstd

