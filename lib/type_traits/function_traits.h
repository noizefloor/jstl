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
#include <tuple>

namespace jstd
{
    template <typename T>
    struct function_traits : public function_traits<decltype(&T::operator())>
    {};

    template <typename ReturnType, typename... Args>
    struct function_traits<ReturnType(*)(Args...)>
    {
        enum { arity = sizeof...(Args) };

        using result_type = ReturnType;

        template <size_t i>
        struct arg
        {
            using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
        };
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType(ClassType::*)(Args...) const>
    {
        enum { arity = sizeof...(Args) };

        using result_type = ReturnType;

        template <size_t i>
        struct arg
        {
            using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
        };
    };

    template <typename ClassType, typename ReturnType, typename... Args>
    struct function_traits<ReturnType(ClassType::*)(Args...)>
    {
        enum { arity = sizeof...(Args) };

        using result_type = ReturnType;

        template <size_t i>
        struct arg
        {
            using type = typename std::tuple_element<i, std::tuple<Args...>>::type;
        };
    };

    template<class F>
    struct function_traits<F&> : public function_traits<F>
    {};

    template<class F>
    struct function_traits<F&&> : public function_traits<F>
    {};

    template <typename T>
    using has_void_return_type = std::is_void<typename function_traits<T>::result_type>;

    namespace internal
    {
        template<typename T>
        struct is_callable_impl
        {
            typedef char yes_type[1];
            typedef char no_type[2];

            template <typename Q>
            static yes_type& check(decltype(&Q::operator())*);

            template <typename Q>
            static no_type& check(...);

            static const bool value = sizeof(check<T>(0))==sizeof(yes_type);
        };

        template<typename R, typename... Args>
        struct is_callable_impl<R (*)(Args...)> : std::true_type {};

        template<typename R, typename C, typename... Args>
        struct is_callable_impl<R (C::*)(Args...)> : std::true_type {};

        template<typename R, typename C, typename... Args>
        struct is_callable_impl<R (C::*)(Args...) const> : std::true_type {};

        template<typename T>
        struct is_callable_impl<T&> : public is_callable_impl<T> {};

        template<typename T>
        struct is_callable_impl<T&&> : public is_callable_impl<T> {};
    }

    template<typename T>
    using is_callable = std::integral_constant<bool, internal::is_callable_impl<T>::value>;
}
