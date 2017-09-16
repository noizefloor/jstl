#pragma once

#include <type_traits>
#include <tuple>

namespace jstd
{
    template <typename T>
    struct function_traits : public function_traits<decltype(&T::operator())>
    {};

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

    template<class F>
    struct function_traits<F&> : public function_traits<F>
    {};

    template<class F>
    struct function_traits<F&&> : public function_traits<F>
    {};
}
