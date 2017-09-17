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
    template <typename ConsumerT,
              typename SourceT = typename conveyor_internal::consumer_type<ConsumerT>::source_type,
              typename ConveyorT = conveyor_internal::conveyor<SourceT, ConsumerT> >
    std::unique_ptr<ConveyorT> make_conveyor(ConsumerT&& consumer)
    {
        static_assert(conveyor_internal::is_consumer<ConsumerT>::value,
                      "The consumer signature is invalid. Expected: 'void(ForwardType&&)'");
        static_assert(std::is_move_constructible<SourceT>::value,
                      "The template parameter is not move constructable. "
                      "If this type cannot be made move constructable use std::unique_ptr<T>.");

        return std::make_unique<ConveyorT>(std::forward<ConsumerT>(consumer));
    };

    template <typename ConverterT, typename... Args,
              typename std::enable_if<!conveyor_internal::is_consumer<ConverterT>::value, int>::type = 0,
              typename SourceT = typename conveyor_internal::converter_type<ConverterT>::source_type>
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


    template <typename ProducerT, typename ConverterOrConsumerT, typename... Args>
    void conveyor_function(ProducerT&& producer, ConverterOrConsumerT&& converterOrConsumer, Args&&... args)
    {
        auto&& conveyor =
                make_conveyor(std::forward<ConverterOrConsumerT>(converterOrConsumer), std::forward<Args>(args)...);
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

