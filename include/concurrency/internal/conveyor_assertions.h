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

#include "conveyor_traits.h"

namespace jstd
{
    namespace internal
    {
        template<typename T>
        struct assert_producer
        {
            static_assert(callable_type<T>::callable == Callable::producer,
                          "Invalid type of parameter.\n"
                          "Expected: void(conveyor_forwarder<'target type'>&)");
        };

        template<typename T>
        struct assert_converter
        {
            static_assert(callable_type<T>::callable == Callable::converter,
                          "Invalid type of parameter.\n"
                          "Expected: void('source type'&&, conveyor_forwarder<'target type'>&)");
        };

        template<typename T>
        struct assert_consumer
        {
            static_assert(callable_type<T>::callable == Callable::consumer,
                          "Invalid type of parameter.\n"
                          "Expected: void(conveyor_forwarder<'source type'>&)");
        };

        template<typename First, typename Second, typename = void>
        struct assert_type_pairs;

        template<typename First, typename Second>
        struct assert_type_pairs<First, Second,
                typename std::enable_if<callable_type<First>::callable == Callable::producer &&
                        callable_type<Second>::callable == Callable::converter,
                        void>::type>
        {
            static_assert(std::is_same<typename callable_type<First>::target_type,
                                       typename callable_type<Second>::source_type>::value,
                          "Types of two successive callable parameters do not match.\n"
                          "Expected: void(conveyor_forwarder<'type'>&), "
                          "void('type'&&, conveyor_forwarder<'target type'>&) ...");
        };

        template<typename First, typename Second>
        struct assert_type_pairs<First, Second,
                typename std::enable_if<callable_type<First>::callable == Callable::converter &&
                        callable_type<Second>::callable == Callable::converter,
                        void>::type>
        {
            static_assert(std::is_same<typename callable_type<First>::target_type,
                                       typename callable_type<Second>::source_type>::value,
                          "Types of two successive callable parameters do not match.\n"
                          "Expected: ... void('source type'&&, conveyor_forwarder<'type'>&), "
                          "void('type'&&, conveyor_forwarder<'target type'>&) ...");
        };

        template<typename First, typename Second>
        struct assert_type_pairs<First, Second,
                typename std::enable_if<callable_type<First>::callable == Callable::converter &&
                        callable_type<Second>::callable == Callable::consumer,
                        void>::type>
        {
            static_assert(std::is_same<typename callable_type<First>::target_type,
                                       typename callable_type<Second>::source_type>::value,
                          "Types of two successive callable parameters do not match.\n"
                          "Expected: ... void('source type'&&, conveyor_forwarder<'type'>&), "
                          "void('type'&&)");
        };

        template<typename First, typename Second>
        struct assert_type_pairs<First, Second,
                typename std::enable_if<callable_type<First>::callable == Callable::producer &&
                                        callable_type<Second>::callable == Callable::consumer, void>::type>
        {
            static_assert(std::is_same<typename callable_type<First>::target_type,
                                       typename callable_type<Second>::source_type>::value,
                          "Types of two successive callable parameters do not match.\n"
                          "Expected: void(conveyor_forwarder<'type'>&), "
                          "void('type'&&)");
        };

        template<typename T>
        struct assert_move_constructible
        {
            static_assert(std::is_move_constructible<typename callable_type<T>::target_type>::value,
                          "The template parameter is not move constructable. "
                          "If this type cannot be made move constructable use std::unique_ptr<T>.");
        };

        template<typename ProducerT, typename... Args>
        struct assert_signature_args;

        template<typename First, typename Second>
        struct assert_signature_args<First, Second> : public assert_converter<First>,
                                                      public assert_consumer<Second>,
                                                      public assert_type_pairs<First, Second>,
                                                      public assert_move_constructible<First>
        {
        };

        template<typename First, typename Second, typename... Args>
        struct assert_signature_args<First, Second, Args...> : public assert_converter<First>,
                                                               public assert_converter<Second>,
                                                               public assert_type_pairs<First, Second>,
                                                               public assert_move_constructible<First>,
                                                               public assert_signature_args<Second, Args...>
        {
        };

        template<typename ProducerT, typename... Args>
        struct assert_signature;

        template<typename First, typename Second>
        struct assert_signature<First, Second> : public assert_producer<First>,
                                                 public assert_consumer<Second>,
                                                 public assert_type_pairs<First, Second>,
                                                 public assert_move_constructible<First>
        {
        };

        template<typename First, typename Second, typename... Args>
        struct assert_signature<First, Second, Args...> : public assert_producer<First>,
                                                          public assert_type_pairs<First, Second>,
                                                          public assert_move_constructible<First>,
                                                          public assert_signature_args<Second, Args...>
        {
        };

    } // internal
} // jstd
