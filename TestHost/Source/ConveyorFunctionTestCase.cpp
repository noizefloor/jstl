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

    jstd::conveyor_function(producer, converter, consumer);

    EXPECT_THAT(results, ElementsAre("A"s, "AA"s, "AAA"s, "AAAA"s, "AAAAA"s));
}


TEST(UnitTest_conveyor_function, is_consumer)
{
    auto function = [](std::string&&) {};
    const auto isConsumer = jstd::internal::is_consumer<decltype(function)>::value;
    EXPECT_TRUE(isConsumer);
}

TEST(UnitTest_conveyor_function, is_consumer_wrongReturnType)
{
    auto function = [](std::string&&) { return 12; };
    const auto isConsumer = jstd::internal::is_consumer<decltype(function)>::value;
    EXPECT_FALSE(isConsumer);
}

TEST(UnitTest_conveyor_function, is_consumer_wrongNumberOfParameters)
{
    auto function = [](std::string&&, int, long) {};
    const auto isConsumer = jstd::internal::is_consumer<decltype(function)>::value;
    EXPECT_FALSE(isConsumer);
}

TEST(UnitTest_conveyor_function, is_consumer_notRvalueRef)
{
    auto function = [](std::string) {};
    const auto isConsumer = jstd::internal::is_consumer<decltype(function)>::value;
    EXPECT_FALSE(isConsumer);
}

TEST(UnitTest_conveyor_function, is_producer)
{
    auto function = [](jstd::conveyor_forwarder<std::string>& forwarder) {};
    const auto isProducer =
            jstd::internal::is_producer<decltype(function),jstd::conveyor_forwarder<std::string>&>::value;
    EXPECT_TRUE(isProducer);
}

TEST(UnitTest_conveyor_function, is_producer_wrongReturnType)
{
    auto function = [](jstd::conveyor_forwarder<std::string>& forwarder) { return 22; };
    const auto isProducer =
            jstd::internal::is_producer<decltype(function), jstd::conveyor_forwarder<std::string>&>::value;
    EXPECT_FALSE(isProducer);
}

TEST(UnitTest_conveyor_function, is_producer_wrongNumberOfParameters)
{
    auto function = [](jstd::conveyor_forwarder<std::string>& forwarder, int, long) {};
    const auto isProducer =
            jstd::internal::is_producer<decltype(function),jstd::conveyor_forwarder<std::string>&>::value;
    EXPECT_FALSE(isProducer);
}

TEST(UnitTest_conveyor_function, is_producer_wrongParameterType)
{
    auto function = [](std::string& forwarder) {};
    const auto isProducer =
            jstd::internal::is_producer<decltype(function), jstd::conveyor_forwarder<std::string>&>::value;
    EXPECT_FALSE(isProducer);
}

TEST(UnitTest_conveyor_function, is_producer_wrongParameterTemplateType)
{
    auto function = [](jstd::conveyor_forwarder<int>& forwarder) {};
    const auto isProducer =
            jstd::internal::is_producer<decltype(function), jstd::conveyor_forwarder<std::string>&>::value;
    EXPECT_FALSE(isProducer);
}

TEST(UnitTest_conveyor_function, is_producer_parameterNotReference)
{
    auto function = [](jstd::conveyor_forwarder<std::string>&& forwarder) {};
    const auto isProducer =
            jstd::internal::is_producer<decltype(function), jstd::conveyor_forwarder<std::string>&>::value;
    EXPECT_FALSE(isProducer);
}

TEST(UnitTest_conveyor_function, is_producer_parameterIsConstRef)
{
    auto function = [](const jstd::conveyor_forwarder<std::string>& forwarder) {};
    const auto isProducer =
            jstd::internal::is_producer<decltype(function),jstd::conveyor_forwarder<std::string>&>::value;
    EXPECT_FALSE(isProducer);
}

TEST(UnitTest_conveyor_function, conveyor_forwarder_type)
{
    using ActualType = jstd::internal::conveyor_forwarder_type<jstd::conveyor_forwarder<std::string> >::type;

    const auto isString = std::is_same<ActualType, std::string>::value;
    EXPECT_TRUE(isString);
}


TEST(UnitTest_conveyor_function, converter_target_variable)
{
    auto converter = [](std::vector<std::string>&&, jstd::conveyor_forwarder<std::string>&) {};
    using TargetType = jstd::internal::converter_target_variable<decltype(converter)>::type;
    const auto isString = std::is_same<TargetType, std::string>::value;
    EXPECT_TRUE(isString);
}
