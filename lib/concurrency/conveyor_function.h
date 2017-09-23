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

#include <type_traits>

#include "internal/conveyor_forwarder.h"
#include "internal/conveyor.h"
#include "internal/conveyor_traits.h"
#include "internal/conveyor_assertions.h"


namespace jstd
{
    namespace internal
    {

        template <typename T,
                  typename SourceType = typename callable_type<T>::source_type,
                  typename ConveyorType = conveyor<SourceType, T> >
        std::unique_ptr<ConveyorType> make_conveyor(T&& consumer)
        {
            return std::make_unique<ConveyorType>(std::forward<T>(consumer));
        };

        template <typename T, typename... Args,
                  typename SourceType = typename callable_type<T>::source_type,
                typename std::enable_if<callable_type<T>::callable == Callable::converter, int>::type = 0>
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
        internal::assert_signature<FirstCallable, SecondCallable, CallableRest...>();

        auto&& conveyor =
                internal::make_conveyor(std::forward<SecondCallable>(converterOrConsumer),
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

