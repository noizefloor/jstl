#include "stdafx.h"

#include <type_traits/function_traits.h>

using namespace jstd;

TEST(UnitTest_FunctionTraits, lambda_returnType)
{
    auto function = [] { return 10; };

    auto isInt = std::is_same<int, function_traits<decltype(function)>::result_type>::value;
    auto isLong = std::is_same<long, function_traits<decltype(function)>::result_type>::value;

    EXPECT_TRUE(isInt);
    EXPECT_FALSE(isLong);
}

TEST(UnitTest_FunctionTraits, lambda_parameterCount_zero)
{
    auto function = [] { return 10; };

    auto parameterCount = function_traits<decltype(function)>::arity;

    EXPECT_EQ(0, parameterCount);
}

TEST(UnitTest_FunctionTraits, lambda_parameterCount_five)
{
    auto function = [] (int, int, long, float, double) { return 10; };

    auto parameterCount = function_traits<decltype(function)>::arity;

    EXPECT_EQ(5, parameterCount);
}

TEST(UnitTest_FunctionTraits, lambda_parameter)
{
    auto function = [] (int, int, long) { return 10; };

    using T = decltype(function);

    auto param0 = std::is_same<int, function_traits<T>::arg<0>::type>::value;
    EXPECT_TRUE(param0);

    auto param1 = std::is_same<int, function_traits<T>::arg<1>::type>::value;
    EXPECT_TRUE(param1);

    auto param2 = std::is_same<long, function_traits<T>::arg<2>::type>::value;
    EXPECT_TRUE(param2);
}

TEST(UnitTest_FunctionTraits, lambda_reference)
{
    auto function = [] { return 10; };
    auto& functionRef = function;

    auto isInt = std::is_same<int, function_traits<decltype(functionRef)>::result_type>::value;

    EXPECT_TRUE(isInt);
}

TEST(UnitTest_FunctionTraits, lambda_constReference)
{
    auto function = [] { return 10; };
    const auto& functionRef = function;

    auto isInt = std::is_same<int, function_traits<decltype(functionRef)>::result_type>::value;

    EXPECT_TRUE(isInt);
}

TEST(UnitTest_FunctionTraits, lambda_rValue)
{
    auto&& function = [] { return 10; };

    auto isInt = std::is_same<int, function_traits<decltype(function)>::result_type>::value;

    EXPECT_TRUE(isInt);
}

TEST(UnitTest_FunctionTraits, std_function_returnType)
{
    std::function<int()> function = [] { return 10; };

    auto isInt = std::is_same<int, function_traits<decltype(function)>::result_type>::value;
    auto isLong = std::is_same<long, function_traits<decltype(function)>::result_type>::value;

    EXPECT_TRUE(isInt);
    EXPECT_FALSE(isLong);
}

TEST(UnitTest_FunctionTraits, std_function_parameterCount_zero)
{
    std::function<int()> function = [] { return 10; };

    auto parameterCount = function_traits<decltype(function)>::arity;

    EXPECT_EQ(0, parameterCount);
}

TEST(UnitTest_FunctionTraits, std_function_parameterCount_five)
{
    std::function<int(int, int, long, float, double)> function =
            [] (int, int, long, float, double) { return 10; };

    auto parameterCount = function_traits<decltype(function)>::arity;

    EXPECT_EQ(5, parameterCount);
}

TEST(UnitTest_FunctionTraits, std_function_parameter)
{
    std::function<int(int, int, long)> function = [] (int, int, long) { return 10; };

    using T = decltype(function);

    auto param0 = std::is_same<int, function_traits<T>::arg<0>::type>::value;
    EXPECT_TRUE(param0);

    auto param1 = std::is_same<int, function_traits<T>::arg<1>::type>::value;
    EXPECT_TRUE(param1);

    auto param2 = std::is_same<long, function_traits<T>::arg<2>::type>::value;
    EXPECT_TRUE(param2);
}

TEST(UnitTest_FunctionTraits, std_function_reference)
{
    std::function<int()> function = [] { return 10; };
    auto& functionRef = function;

    auto isInt = std::is_same<int, function_traits<decltype(functionRef)>::result_type>::value;

    EXPECT_TRUE(isInt);
}

TEST(UnitTest_FunctionTraits, std_function_constReference)
{
    std::function<int()> function = [] { return 10; };
    const auto& functionRef = function;

    auto isInt = std::is_same<int, function_traits<decltype(functionRef)>::result_type>::value;

    EXPECT_TRUE(isInt);
}

TEST(UnitTest_FunctionTraits, std_function_rValue)
{
    std::function<int()> function = [] { return 10; };

    auto&& functionRvalue = std::move(function);

    auto isInt = std::is_same<int, function_traits<decltype(functionRvalue)>::result_type>::value;

    EXPECT_TRUE(isInt);
}
