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
    class conveyor_forwarder;


    template<typename T>
    using conveyor_consumer = std::function<void(T&&)>;

    template<typename T>
    using conveyor_producer = std::function<void(conveyor_forwarder<T>&)>;


    template <typename ForwardT>
    class conveyor_forwarder
    {
    public:
        explicit conveyor_forwarder(internal::conveyor_forwarder<ForwardT>& forwarder)
            : _forwarder(forwarder)
        {
        }

        void push(ForwardT&& forwardValue)
        {
            _forwarder.push(std::move(forwardValue));
        }

        template<typename T = ForwardT>
        void push(const T& forwardValue,
                  typename std::enable_if<std::is_copy_constructible<T>::value>::type* = 0)
        {
            _forwarder.push(std::move(forwardValue));
        }

    private:
        internal::conveyor_forwarder<ForwardT>& _forwarder;
    };


    template <typename ForwardT,
              typename ProducerT = conveyor_producer<ForwardT>,
              typename ConsumerT = conveyor_consumer<ForwardT> >
    void conveyor_function(ProducerT&& producer,
                           ConsumerT&& consumer)
    {
        static_assert(std::is_move_constructible<ForwardT>::value,
                      "The template parameter is not move constructable. "
                              "If this type cannot be made move constructable use std::unique_ptr<T>.");

        auto&& internalForwarder = internal::conveyor_forwarder<ForwardT>(std::forward<ConsumerT>(consumer));

        try
        {
            auto&& forwarder = conveyor_forwarder<ForwardT>(internalForwarder);
            producer(forwarder);
        }
        catch (...)
        {
            internalForwarder.finish();
            throw;
        }

        internalForwarder.finish();
        internalForwarder.checkForError();
    }

} // jstd

