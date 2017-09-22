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
#include "../../type_traits/function_traits.h"

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

    namespace conveyor_internal
    {
        enum class CallableType
        {
            unknown = 0,
            producer = 1,
            converter,
            consumer
        };

        struct no_callable_type
        {
            static const CallableType callable = CallableType::unknown;

            using source_type = void;
            using target_type = void;
        };

        template <typename T>
        struct callable_type_impl : public callable_type_impl<decltype(&T::operator())> {};

        template <typename ReturnType, typename ClassType, typename... Args>
        struct callable_type_impl<ReturnType(ClassType::*)(Args...)>
                : public callable_type_impl<ReturnType(Args...)> {};

        template <typename ReturnType, typename ClassType, typename... Args>
        struct callable_type_impl<ReturnType(ClassType::*)(Args...) const>
                : public callable_type_impl<ReturnType(Args...)> {};

        template <typename ReturnType, typename... Args>
        struct callable_type_impl<ReturnType(*)(Args...)> : public callable_type_impl<ReturnType(Args...)> {};


        template <typename ReturnType, typename... Args>
        struct callable_type_impl<ReturnType(Args...)> : public no_callable_type
        {};

        template <typename T>
        struct callable_type_impl<T&> : public callable_type_impl<T> {};

        template <typename T>
        struct callable_type_impl<T&&> : public callable_type_impl<T> {};

        template <typename TargetType>
        struct callable_type_impl<void(conveyor_forwarder<TargetType>&)>
        {
            static const CallableType callable = CallableType::producer;

            using source_type = void;
            using target_type = TargetType;
        };

        template <typename SourceType, typename TargetType>
        struct callable_type_impl<void(SourceType&&, conveyor_forwarder<TargetType>&)>
        {
            static const CallableType callable = CallableType::converter;

            using source_type = SourceType;
            using target_type = TargetType;
        };

        template <typename SourceType>
        struct callable_type_impl<void(SourceType&&)>
        {
            static const CallableType callable = CallableType::consumer;

            using source_type = SourceType;
            using target_type = void;
        };

        template <typename T, typename = void>
        struct callable_type : public no_callable_type {};

        template <typename T>
        struct callable_type<T, typename std::enable_if<is_callable<T>::value>::type >
                : public callable_type_impl<T> {};




        template <typename ValueT>
        struct forwarder_type
        {
            static const bool is_forwarder = false;
            using type = void;
        };

        template <typename ValueT>
        struct forwarder_type<conveyor_forwarder<ValueT> >
        {
            static const bool is_forwarder = true;
            using type = ValueT;
        };

        template <typename ValueT>
        struct forwarder_type<ValueT&> : public forwarder_type<ValueT> {};

        template <typename ValueT>
        struct forwarder_type<ValueT&&> : public forwarder_type<ValueT> {};



        template <typename T>
        struct assert_producer
        {
            static_assert(callable_type<T>::callable == CallableType::producer,
                          "Invalid type of parameter. Expected: void(conveyor_forwarder<'target type'>&)");
        };

        template <typename T>
        struct assert_converter
        {
            static_assert(callable_type<T>::callable == CallableType::converter,
                          "Invalid type of parameter. Expected: void('source type'&&, conveyor_forwarder<'target type'>&)");
        };

        template <typename T>
        struct assert_consumer
        {
            static_assert(callable_type<T>::callable == CallableType::consumer,
                          "Invalid type of parameter. Expected: void(conveyor_forwarder<'source type'>&)");
        };

        template <typename First, typename Second, typename = void>
        struct assert_type_pairs;

        template <typename First, typename Second>
        struct assert_type_pairs<First, Second,
                                 typename std::enable_if<callable_type<First>::callable == CallableType::producer &&
                                                         callable_type<Second>::callable == CallableType::converter,
                                                         void>::type>
        {
            static_assert(std::is_same<typename callable_type<First>::target_type, typename callable_type<Second>::source_type>::value,
                          "Types of two successive callable parameters do not match.\n"
                          "Expected: void(conveyor_forwarder<'type'>&), "
                          "void('type'&&, conveyor_forwarder<'target type'>&) ...");
        };

        template <typename First, typename Second>
        struct assert_type_pairs<First, Second,
                                 typename std::enable_if<callable_type<First>::callable == CallableType::converter &&
                                                         callable_type<Second>::callable == CallableType::converter,
                                                         void>::type>
        {
            static_assert(std::is_same<typename callable_type<First>::target_type, typename callable_type<Second>::source_type>::value,
                          "Types of two successive callable parameters do not match.\n"
                          "Expected: ... void('source type'&&, conveyor_forwarder<'type'>&), "
                          "void('type'&&, conveyor_forwarder<'target type'>&) ...");
        };

        template <typename First, typename Second>
        struct assert_type_pairs<First, Second,
                                 typename std::enable_if<callable_type<First>::callable == CallableType::converter &&
                                                         callable_type<Second>::callable == CallableType::consumer,
                                                         void>::type>
        {
            static_assert(std::is_same<typename callable_type<First>::target_type, typename callable_type<Second>::source_type>::value,
                          "Types of two successive callable parameters do not match.\n"
                          "Expected: ... void('source type'&&, conveyor_forwarder<'type'>&), "
                          "void('type'&&)");
        };

        template <typename First, typename Second>
        struct assert_type_pairs<First, Second,
                                 typename std::enable_if<callable_type<First>::callable == CallableType::producer &&
                                                         callable_type<Second>::callable == CallableType::consumer,
                                                         void>::type>
        {
            static_assert(std::is_same<typename callable_type<First>::target_type, typename callable_type<Second>::source_type>::value,
                          "Types of two successive callable parameters do not match.\n"
                                  "Expected: void(conveyor_forwarder<'type'>&), "
                                  "void('type'&&)");
        };

        template <typename ProducerT, typename... Args>
        struct assert_signature_args;

        template <typename First, typename Second>
        struct assert_signature_args<First, Second> : public assert_converter<First>,
                                                      public assert_consumer<Second>,
                                                      public assert_type_pairs<First, Second> {};

        template <typename First, typename Second, typename... Args>
        struct assert_signature_args<First, Second, Args...> : public assert_converter<First>,
                                                               public assert_converter<Second>,
                                                               public assert_type_pairs<First, Second>,
                                                               public assert_signature_args<Second, Args...> {};

        template <typename ProducerT, typename... Args>
        struct assert_signature;

        template <typename First, typename Second>
        struct assert_signature<First, Second> : public assert_producer<First>,
                                                 public assert_consumer<Second>,
                                                 public assert_type_pairs<First, Second> {};

        template <typename First, typename Second, typename... Args>
        struct assert_signature<First, Second, Args...> : public assert_producer<First>,
                                                          public assert_type_pairs<First, Second>,
                                                          public assert_signature_args<Second, Args...>
        {
        };

    } // conveyor_internal

} // jstd