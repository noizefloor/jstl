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

namespace jstd
{
    template <typename ForwardT>
    class conveyor_forwarder
    {
    public:
        virtual ~conveyor_forwarder() = default;

        virtual void push(ForwardT&& forwardValue) = 0;
        virtual void push(const ForwardT& forwardValue) = 0;
    };

    template <typename ForwardT, typename ConsumerT>
    class conveyor_forwarder_impl : public jstd::conveyor_forwarder<ForwardT>
    {
    public:
        explicit conveyor_forwarder_impl(internal::conveyor<ForwardT, ConsumerT>& forwarder)
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
        internal::conveyor<ForwardT, ConsumerT>& _forwarder;
    };

    template <typename ProducerT,
              typename ConsumerT,
              typename ForwardT = typename internal::consumer_variable<ConsumerT>::type >
    void conveyor_function(ProducerT&& producer, ConsumerT&& consumer)
    {
        static_assert(internal::is_consumer<ConsumerT>::value,
                      "The consumer signature is invalid. Expected: 'void(ForwardType&&)'");
        static_assert(internal::is_producer<ProducerT, conveyor_forwarder<ForwardT>&>::value,
                      "The producer signature is invalid. Expected: 'void(conveyor_forwarder<ForwardType>&)'");
        static_assert(std::is_move_constructible<ForwardT>::value,
                      "The template parameter is not move constructable. "
                      "If this type cannot be made move constructable use std::unique_ptr<T>.");


        auto&& conveyor = internal::conveyor<ForwardT, ConsumerT>(std::forward<ConsumerT>(consumer));

        try
        {
            auto&& forwarder = conveyor_forwarder_impl<ForwardT, ConsumerT>(conveyor);
            producer(forwarder);
        }
        catch (...)
        {
            conveyor.finish();
            throw;
        }

        conveyor.finish();
        conveyor.checkForError();
    };

} // jstd

