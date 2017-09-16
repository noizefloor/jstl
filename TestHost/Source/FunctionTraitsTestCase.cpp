#include "stdafx.h"

using namespace ::testing;

#include <type_traits/function_traits.h>

using namespace jstd;

TEST(UnitTest_FunctionTraits, lambda_returnType)
{
    auto function = [] { return 10; };

    EXPECT_THAT(function(), A<function_traits<decltype(function)>::result_type>());
}

TEST(UnitTest_FunctionTraits, lambda_parameterCount_zero)
{
    auto function = [] { return 10; };

    const auto parameterCount = function_traits<decltype(function)>::arity;

    EXPECT_EQ(0, parameterCount);
}

TEST(UnitTest_FunctionTraits, lambda_parameterCount_five)
{
    auto function = [] (int, int, long, float, double) { return 10; };

    const auto parameterCount = function_traits<decltype(function)>::arity;

    EXPECT_EQ(5, parameterCount);
}

TEST(UnitTest_FunctionTraits, lambda_parameter)
{
    auto function = [] (int, int, long) { return 10; };

    using T = decltype(function);

    auto param0 = std::is_same<int, function_traits<T>::arg<0>::type>::value;
    EXPECT_TRUE(param0) << "arg<0> is not an int";

    auto param1 = std::is_same<int, function_traits<T>::arg<1>::type>::value;
    EXPECT_TRUE(param1) << "arg<1> is not an int";

    auto param2 = std::is_same<long, function_traits<T>::arg<2>::type>::value;
    EXPECT_TRUE(param2) << "arg<2> is not an long";
}

TEST(UnitTest_FunctionTraits, lambda_reference)
{
    auto function = [] { return 10; };
    auto& functionRef = function;

    EXPECT_THAT(function(), A<function_traits<decltype(functionRef)>::result_type>());
}

TEST(UnitTest_FunctionTraits, lambda_constReference)
{
    auto function = [] { return 10; };
    const auto& functionRef = function;

    EXPECT_THAT(function(), A<function_traits<decltype(functionRef)>::result_type>());
}

TEST(UnitTest_FunctionTraits, lambda_rValue)
{
    auto&& function = [] { return 10; };

    EXPECT_THAT(function(), A<function_traits<decltype(function)>::result_type>());
}

TEST(UnitTest_FunctionTraits, std_function_returnType)
{
    std::function<int()> function = [] { return 10; };

    EXPECT_THAT(function(), A<function_traits<decltype(function)>::result_type>());
}

TEST(UnitTest_FunctionTraits, std_function_parameterCount_zero)
{
    std::function<int()> function = [] { return 10; };

    const auto parameterCount = function_traits<decltype(function)>::arity;

    EXPECT_EQ(0, parameterCount);
}

TEST(UnitTest_FunctionTraits, std_function_parameterCount_five)
{
    std::function<int(int, int, long, float, double)> function =
            [] (int, int, long, float, double) { return 10; };

    const auto parameterCount = function_traits<decltype(function)>::arity;

    EXPECT_EQ(5, parameterCount);
}

TEST(UnitTest_FunctionTraits, std_function_parameter)
{
    std::function<int(int, int, long)> function = [] (int, int, long) { return 10; };

    using T = decltype(function);

    auto param0 = std::is_same<int, function_traits<T>::arg<0>::type>::value;
    EXPECT_TRUE(param0) << "arg<0> is not an int";

    auto param1 = std::is_same<int, function_traits<T>::arg<1>::type>::value;
    EXPECT_TRUE(param1) << "arg<1> is not an int";

    auto param2 = std::is_same<long, function_traits<T>::arg<2>::type>::value;
    EXPECT_TRUE(param2) << "arg<0> is not an long";
}

TEST(UnitTest_FunctionTraits, std_function_reference)
{
    std::function<int()> function = [] { return 10; };
    auto& functionRef = function;

    EXPECT_THAT(function(), A<function_traits<decltype(functionRef)>::result_type>());
}

TEST(UnitTest_FunctionTraits, std_function_constReference)
{
    std::function<int()> function = [] { return 10; };
    const auto& functionRef = function;

    EXPECT_THAT(function(), A<function_traits<decltype(functionRef)>::result_type>());
}

TEST(UnitTest_FunctionTraits, std_function_rValue)
{
    std::function<int()> function = [] { return 10; };

    auto&& functionRvalue = std::move(function);

    EXPECT_THAT(function(), A<function_traits<decltype(functionRvalue)>::result_type>());
}

namespace
{
    int testFunction(int, long, double)
    {
        return 10;
    }
}

TEST(UnitTest_FunctionTraits, function_returnType)
{
    auto result = testFunction(1, 2, 3.);

    EXPECT_THAT(result, testing::A<function_traits<decltype(&testFunction)>::result_type>());
}

TEST(UnitTest_FunctionTraits, function_parameterCount)
{
    auto parameterCount = function_traits<decltype(&testFunction)>::arity;

    EXPECT_EQ(3, parameterCount);
}

TEST(UnitTest_FunctionTraits, function_parameter)
{
    using T = decltype(&testFunction);

    auto param0 = std::is_same<int, function_traits<T>::arg<0>::type>::value;
    EXPECT_TRUE(param0) << "arg<0> is not an int";

    auto param1 = std::is_same<long, function_traits<T>::arg<1>::type>::value;
    EXPECT_TRUE(param1) << "arg<1> is not a long";

    auto param2 = std::is_same<double, function_traits<T>::arg<2>::type>::value;
    EXPECT_TRUE(param2) << "arg<2> is not a double";
}

namespace
{
    class TestClass
    {
    public:
        int testFunction(int, long, double)
        {
            return 10;
        }

        int testConstFunction(int, long, double) const
        {
            return 10;
        }
    };
}

TEST(UnitTest_FunctionTraits, memberFunction_returnType)
{
    auto&& testClass = TestClass();

    auto result = testClass.testFunction(1, 2, 3.);

    EXPECT_THAT(result, testing::A<function_traits<decltype(&TestClass::testFunction)>::result_type>());
}

TEST(UnitTest_FunctionTraits, memberFunction_parameterCount)
{
    auto parameterCount = function_traits<decltype(&TestClass::testFunction)>::arity;

    EXPECT_EQ(3, parameterCount);
}

TEST(UnitTest_FunctionTraits, memberFunction_parameter)
{
    using T = decltype(&TestClass::testFunction);

    auto param0 = std::is_same<int, function_traits<T>::arg<0>::type>::value;
    EXPECT_TRUE(param0) << "arg<0> is not an int";

    auto param1 = std::is_same<long, function_traits<T>::arg<1>::type>::value;
    EXPECT_TRUE(param1) << "arg<1> is not a long";

    auto param2 = std::is_same<double, function_traits<T>::arg<2>::type>::value;
    EXPECT_TRUE(param2) << "arg<2> is not a double";
}

TEST(UnitTest_FunctionTraits, constMemberFunction_returnType)
{
    auto&& testClass = TestClass();

    auto result = testClass.testConstFunction(1, 2, 3.);

    EXPECT_THAT(result, testing::A<function_traits<decltype(&TestClass::testConstFunction)>::result_type>());
}

TEST(UnitTest_FunctionTraits, constMemberFunctionparameterCount)
{
    auto parameterCount = function_traits<decltype(&TestClass::testConstFunction)>::arity;

    EXPECT_EQ(3, parameterCount);
}

TEST(UnitTest_FunctionTraits, constMemberFunctionparameter)
{
    using T = decltype(&TestClass::testConstFunction);

    auto param0 = std::is_same<int, function_traits<T>::arg<0>::type>::value;
    EXPECT_TRUE(param0) << "arg<0> is not an int";

    auto param1 = std::is_same<long, function_traits<T>::arg<1>::type>::value;
    EXPECT_TRUE(param1) << "arg<1> is not a long";

    auto param2 = std::is_same<double, function_traits<T>::arg<2>::type>::value;
    EXPECT_TRUE(param2) << "arg<2> is not a double";
}
