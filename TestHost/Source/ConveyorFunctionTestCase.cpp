#include "stdafx.h"

#include <concurrency/conveyor_function.h>

using namespace std::literals::string_literals;
using testing::ElementsAre;

TEST(UnitTest_conveyor_function, pushByMove)
{
    auto&& results = std::vector<std::string>();

    jstd::conveyor_function([](jstd::conveyor_forwarder<std::string>& forwarder)
                            {
                                forwarder.push("value1"s);
                                forwarder.push("value2"s);
                                forwarder.push("value3"s);
                                forwarder.push("value4"s);
                                forwarder.push("value5"s);
                            },
                            [&](std::string&& value)
                            {
                                results.push_back(std::move(value));
                            });



    EXPECT_THAT(results, ElementsAre("value1"s, "value2"s, "value3"s, "value4"s, "value5"s));
}

TEST(UnitTest_conveyor_function, pushByCopy)
{
    auto&& results = std::vector<std::string>();

    auto&& values = std::vector<std::string>{ "value1"s, "value2"s, "value3"s, "value4"s, "value5"s };

    jstd::conveyor_function([&](jstd::conveyor_forwarder<std::string>& forwarder)
                            {
                                for (const auto& value : values)
                                    forwarder.push(value);
                            },
                            [&](std::string&& value)
                            {
                                results.push_back(std::move(value));
                            });

    EXPECT_THAT(results, ElementsAre("value1"s, "value2"s, "value3"s, "value4"s, "value5"s));
}

TEST(UnitTest_conveyor_function, copyFirstFunctions)
{
    auto&& results = std::vector<std::string>();

    const auto producer = [](jstd::conveyor_forwarder<std::string>& forwarder)
    {
        forwarder.push("value1"s);
        forwarder.push("value2"s);
        forwarder.push("value3"s);
    };

    jstd::conveyor_function(producer,
                            [&](std::string&& value)
                            {
                                results.push_back(std::move(value));
                            });

    EXPECT_THAT(results, ElementsAre("value1"s, "value2"s, "value3"s));
}

TEST(UnitTest_conveyor_function, copySecondFunctions)
{
    auto&& results = std::vector<std::string>();

    auto consumer = [&](std::string&& value) { results.push_back(std::move(value)); };

    jstd::conveyor_function([](jstd::conveyor_forwarder<std::string>& forwarder)
                            {
                                forwarder.push("value1"s);
                                forwarder.push("value2"s);
                                forwarder.push("value3"s);
                            },
                            consumer);

    EXPECT_THAT(results, ElementsAre("value1"s, "value2"s, "value3"s));
}

TEST(UnitTest_conveyor_function, copyBothFunctions)
{
    auto&& results = std::vector<std::string>();

    auto producer = [](jstd::conveyor_forwarder<std::string>& forwarder)
    {
        forwarder.push("value1"s);
        forwarder.push("value2"s);
        forwarder.push("value3"s);
    };
    auto consumer = [&](std::string&& value) { results.push_back(std::move(value)); };

    jstd::conveyor_function(producer, consumer);

    EXPECT_THAT(results, ElementsAre("value1"s, "value2"s, "value3"s));
}

class TestException : public std::runtime_error
{
public:
    TestException()
        : runtime_error("TestError")
    {
    }
};

TEST(UnitTest_conveyor_function, producerThrows)
{
    auto&& results = std::vector<std::string>();

    auto producer = [](jstd::conveyor_forwarder<std::string>& forwarder)
    {
        forwarder.push("value1"s);
        forwarder.push("value2"s);
        throw TestException();
    };

    auto consumer = [&](std::string&& value) { results.push_back(std::move(value)); };

    EXPECT_THROW(jstd::conveyor_function(std::move(producer), std::move(consumer)), TestException);
}

TEST(UnitTest_conveyor_function, consumerThrowsWhileProducing)
{
    std::atomic_int processed(0);

    auto producer = [&](jstd::conveyor_forwarder<std::string>& forwarder)
    {
        while(processed < 20)
            forwarder.push("value"s);
    };

    auto consumer = [&](std::string&& value)
    {
        if (processed++ > 10)
            throw TestException();
    };

    EXPECT_THROW(jstd::conveyor_function(std::move(producer), std::move(consumer)), TestException);
}

TEST(UnitTest_conveyor_function, consumerThrowsWhileWaiting)
{
    std::atomic_int processed(0);

    auto producer = [&](jstd::conveyor_forwarder<std::string>& forwarder)
    {
        for(; processed < 30; ++processed)
            forwarder.push("value"s);
    };

    auto consumer = [&](std::string&& value)
    {
        if (processed >= 30)
        {
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            throw TestException();
        }
    };

    EXPECT_THROW(jstd::conveyor_function(std::move(producer), std::move(consumer)), TestException);
}

TEST(UnitTest_conveyor_function, pipeline_simple)
{
    auto results = std::vector<std::string>();

    auto producer = [](jstd::conveyor_forwarder<std::string>& forwarder)
    {
        for (auto i = size_t(1); i <= 5; ++i)
        {
            auto&& stringProducer = std::string(i, 'A');
            forwarder.push(std::move(stringProducer));
        }
    };

    auto converter = [](std::string&& value, jstd::conveyor_forwarder<std::string>& forwarder)
    {
        value += "_B";
        forwarder.push(std::move(value));
    };

    auto consumer = [&](std::string&& value)
    {
        results.push_back(std::move(value));
    };

    jstd::conveyor_function(producer, converter, consumer);

    EXPECT_THAT(results, ElementsAre("A_B"s, "AA_B"s, "AAA_B"s, "AAAA_B"s, "AAAAA_B"s));
}

class StringProducer
{
public:
    StringProducer(StringProducer&&) = default;
    StringProducer& operator=(StringProducer&&) = default;

    StringProducer(const StringProducer&) = default;
    StringProducer& operator=(const StringProducer&) = default;

    StringProducer(size_t count)
        : _text(count, 'A')
    {
    }

    std::string toString() const
    {
        return _text;
    }

private:
    std::string _text;
};

TEST(UnitTest_conveyor_function, pipeline)
{
    auto results = std::vector<std::string>();

    auto producer = [](jstd::conveyor_forwarder<StringProducer>& forwarder)
    {
        for (auto i = size_t(1); i <= 5; ++i)
        {
            auto&& stringProducer = StringProducer(i);
            forwarder.push(std::move(stringProducer));
        }
    };

    auto converter = [](StringProducer&& value, jstd::conveyor_forwarder<std::string>& forwarder)
    {
        auto forwardValue = value.toString();
        forwarder.push(std::move(forwardValue));
    };

    auto consumer = [&](std::string&& value)
    {
        results.push_back(std::move(value));
    };

    jstd::conveyor_function(std::move(producer), std::move(converter), std::move(consumer));

    EXPECT_THAT(results, ElementsAre("A"s, "AA"s, "AAA"s, "AAAA"s, "AAAAA"s));
}

TEST(UnitTest_conveyor_function, pipeline_consumer_throws)
{
    auto results = std::vector<std::string>();

    auto producer = [](jstd::conveyor_forwarder<std::string>& forwarder)
    {
        for (auto i = size_t(1); i <= 5; ++i)
        {
            auto&& stringProducer = std::string(i, 'A');
            forwarder.push(std::move(stringProducer));
        }
    };

    auto converter = [](std::string&& value, jstd::conveyor_forwarder<std::string>& forwarder)
    {
        value += "_B";
        forwarder.push(std::move(value));
    };

    auto i = 0;

    auto consumer = [&](std::string&& value)
    {
        if (i++ == 3)
            throw TestException();

        results.push_back(std::move(value));
    };

    EXPECT_THROW(jstd::conveyor_function(producer, converter, consumer), TestException);
}

TEST(UnitTest_conveyor_function, pipeline_converter_throws)
{
    auto results = std::vector<std::string>();

    auto producer = [](jstd::conveyor_forwarder<std::string>& forwarder)
    {
        for (auto i = size_t(1); i <= 5; ++i)
        {
            auto&& stringProducer = std::string(i, 'A');
            forwarder.push(std::move(stringProducer));
        }
    };

    auto i = 0;
    auto converter = [&](std::string&& value, jstd::conveyor_forwarder<std::string>& forwarder)
    {
        if (i++ == 3)
            throw TestException();

        value += "_B";
        forwarder.push(std::move(value));
    };

    auto consumer = [&](std::string&& value)
    {
        if (i++ == 3)
            throw TestException();

        results.push_back(std::move(value));
    };

    EXPECT_THROW(jstd::conveyor_function(producer, converter, consumer), TestException);
}

TEST(UnitTest_conveyor_function, forwarder_type)
{
    using ForwarderType = jstd::conveyor_forwarder<std::string>;

    const auto isForwarder = jstd::conveyor_internal::forwarder_type<ForwarderType>::is_forwarder;
    EXPECT_TRUE(isForwarder);

    using ActualType = jstd::conveyor_internal::forwarder_type<ForwarderType>::type;
    const auto isString = std::is_same<ActualType, std::string>::value;
    EXPECT_TRUE(isString);
}

TEST(UnitTest_conveyor_function, forwarder_type_wrongType)
{
    const auto isForwarder = jstd::conveyor_internal::forwarder_type<std::string>::is_forwarder;
    EXPECT_FALSE(isForwarder);

    using ActualType = jstd::conveyor_internal::forwarder_type<std::string>::type;
    const auto isString = std::is_same<ActualType, std::string>::value;
    EXPECT_FALSE(isString);
}

TEST(UnitTest_conveyor_function, callable_type_unknown_lambda)
{
    auto input = [](std::string&) {};

    using CallableType = jstd::conveyor_internal::callable_type<decltype(input)>;

    const auto callableType = CallableType::callable;
    EXPECT_EQ(jstd::conveyor_internal::CallableType::unknown, callableType);
}

TEST(UnitTest_conveyor_function, callable_type_unknown_variable)
{
    auto input = std::string();

    using CallableType = jstd::conveyor_internal::callable_type<decltype(input)>;

    const auto callableType = CallableType::callable;
    EXPECT_EQ(jstd::conveyor_internal::CallableType::unknown, callableType);
}

TEST(UnitTest_conveyor_function, callable_type_unknown_return_type)
{
    auto input = [](jstd::conveyor_forwarder<std::string>&) { return 0; };

    using CallableType = jstd::conveyor_internal::callable_type<decltype(input)>;

    const auto callableType = CallableType::callable;
    EXPECT_EQ(jstd::conveyor_internal::CallableType::unknown, callableType);
}

TEST(UnitTest_conveyor_function, callable_type_producer)
{
    auto input = [](jstd::conveyor_forwarder<std::string>&) {};

    using CallableType = jstd::conveyor_internal::callable_type<decltype(input)>;

    const auto callableType = CallableType::callable;
    EXPECT_EQ(jstd::conveyor_internal::CallableType::producer, callableType);

    const auto isTargetValid = std::is_same<CallableType::target_type, std::string >::value;
    EXPECT_TRUE(isTargetValid) << "converter_type::source is not std::string";
}

TEST(UnitTest_conveyor_function, callable_type_converter)
{
    auto input = [](std::vector<std::string>&&, jstd::conveyor_forwarder<std::string>&) {};

    using CallableType = jstd::conveyor_internal::callable_type<decltype(input)>;

    const auto callableType = CallableType::callable;
    EXPECT_EQ(jstd::conveyor_internal::CallableType::converter, callableType);

    const auto isSourceValid = std::is_same<CallableType::source_type, std::vector<std::string> >::value;
    EXPECT_TRUE(isSourceValid) << "converter_type::source is not std::vector<std::string>";

    const auto isTargetValid = std::is_same<CallableType::target_type, std::string >::value;
    EXPECT_TRUE(isTargetValid) << "converter_type::source is not std::string";
}

TEST(UnitTest_conveyor_function, callable_type_consumer)
{
    auto input = [](std::vector<std::string>&&) {};

    using CallableType = jstd::conveyor_internal::callable_type<decltype(input)>;

    const auto callableType = CallableType::callable;
    EXPECT_EQ(jstd::conveyor_internal::CallableType::consumer, callableType);

    const auto isSourceValid = std::is_same<CallableType::source_type, std::vector<std::string> >::value;
    EXPECT_TRUE(isSourceValid) << "converter_type::source is not std::vector<std::string>";
}

