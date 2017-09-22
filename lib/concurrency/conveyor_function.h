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

        template<typename ForwardT, typename ConsumerT>
        class conveyor : public conveyor_proxy
        {
            class conveyor_forwarder_impl : public jstd::conveyor_forwarder<ForwardT>
            {
            public:
                explicit conveyor_forwarder_impl(conveyor<ForwardT, ConsumerT>& forwarder)
                        : _forwarder(forwarder)
                {
                }

                virtual ~conveyor_forwarder_impl() = default;

                void push(ForwardT&& forwardValue) override
                {
                    _forwarder.push(std::move(forwardValue));
                }

                void push(const ForwardT& forwardValue) override
                {
                    static_assert(std::is_copy_constructible<ForwardT>::value,
                                  "Pushing a non copyable type by reference is not supported. Consider using std::move.");
                    _forwarder.push(forwardValue);
                }

            private:
                conveyor<ForwardT, ConsumerT>& _forwarder;
            };

        public:
            explicit conveyor(ConsumerT&& consumer)
                    : _consumer(std::forward<ConsumerT>(consumer))
                      , _forwarder(*this)
                      , _consumerHandle(std::async(std::launch::async, [this] { run(); }))
            {
            }

            virtual ~conveyor() = default;

            template<typename T>
            void push(T&& forwardValue)
            {
                checkForError();

                std::unique_lock<std::mutex> lock(_mutex);

                _queue.push(std::forward<T>(forwardValue));

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

                if (_hasError)
                    std::rethrow_exception(_error);
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
            ConsumerT _consumer;

            std::queue<ForwardT> _queue;
            std::mutex _mutex;
            std::condition_variable _cv;

            std::atomic_bool _shouldFinish { false };

            conveyor_forwarder_impl _forwarder;
            std::future<void> _consumerHandle;

            std::exception_ptr _error;
            std::atomic_bool _hasError { false };
        };

        template <typename ConsumerT,
                  typename SourceT = typename callable_type<ConsumerT>::source_type,
                  typename ConveyorT = conveyor<SourceT, ConsumerT> >
        std::unique_ptr<ConveyorT> make_conveyor(ConsumerT&& consumer)
        {
            static_assert(callable_type<ConsumerT>::callable == CallableType::consumer,
                          "The consumer signature is invalid. Expected: 'void(ForwardType&&)'");
            static_assert(std::is_move_constructible<SourceT>::value,
                          "The template parameter is not move constructable. "
                                  "If this type cannot be made move constructable use std::unique_ptr<T>.");

            return std::make_unique<ConveyorT>(std::forward<ConsumerT>(consumer));
        };

        template <typename ConverterT, typename... Args,
                  typename SourceT = typename callable_type<ConverterT>::source_type,
                typename std::enable_if<callable_type<ConverterT>::callable == CallableType::converter, int>::type = 0>
        auto make_conveyor(ConverterT&& converter, Args&&... args)
        {
            auto&& conveyor = make_conveyor(std::forward<Args>(args)...);
            auto& forwarder = conveyor->getForwarder();

            auto&& consumer = [&forwarder, cv = std::forward<ConverterT>(converter)](SourceT&& value)
            {
                cv(std::move(value), forwarder);
            };

            auto&& resultConveyor = make_conveyor(std::move(consumer));
            resultConveyor->setConveyorProxy(std::move(conveyor));

            return std::move(resultConveyor);
        };
    }


    template <typename ProducerT, typename ConverterOrConsumerT, typename... Args>
    void conveyor_function(ProducerT&& producer, ConverterOrConsumerT&& converterOrConsumer, Args&&... args)
    {
        auto&& conveyor =
                conveyor_internal::make_conveyor(std::forward<ConverterOrConsumerT>(converterOrConsumer),
                                                 std::forward<Args>(args)...);
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

