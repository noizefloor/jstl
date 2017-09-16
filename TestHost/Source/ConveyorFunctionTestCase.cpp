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

    auto producer = [](jstd::conveyor_forwarder<std::string>& forwarder)
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
