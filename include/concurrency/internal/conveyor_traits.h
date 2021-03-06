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

#include "../../type_traits/function_traits.h"
#include "conveyor_forwarder.h"

namespace jstd
{
    namespace internal
    {
        enum class Callable
        {
            unknown = 0,
            producer = 1,
            converter,
            consumer
        };

        struct no_callable_type
        {
            static const Callable callable = Callable::unknown;

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
            static const Callable callable = Callable::producer;

            using source_type = void;
            using target_type = TargetType;
        };

        template <typename SourceType, typename TargetType>
        struct callable_type_impl<void(SourceType&&, conveyor_forwarder<TargetType>&)>
        {
            static const Callable callable = Callable::converter;

            using source_type = SourceType;
            using target_type = TargetType;
        };

        template <typename SourceType>
        struct callable_type_impl<void(SourceType&&)>
        {
            static const Callable callable = Callable::consumer;

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

    } // internal

} // jstd