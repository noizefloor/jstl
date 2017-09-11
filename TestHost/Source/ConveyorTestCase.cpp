#include "stdafx.h"

#include <concurrency/conveyor.h>

#include "PerformanceCounter.h"

using jstd::conveyor;
using testing::ElementsAre;
using namespace std::literals::string_literals;

TEST(UnitTest_conveyor, pushByMoveAndWait)
{
    auto&& results = std::vector<std::string>();

    {
        auto&& testConveyor =
                conveyor<std::string>( [&](std::string&& value) { results.push_back(std::move(value)); });

        testConveyor.push("value1"s);
        testConveyor.push("value2"s);
        testConveyor.push("value3"s);
        testConveyor.push("value4"s);
        testConveyor.push("value5"s);
    }

    EXPECT_THAT(results, ElementsAre("value1"s, "value2"s, "value3"s, "value4"s, "value5"s));
}

TEST(UnitTest_conveyor, pushByCopyAndWait)
{
    auto&& results = std::vector<std::string>();

    const auto value1 = "value1"s;
    const auto value2 = "value2"s;
    const auto value3 = "value3"s;
    const auto value4 = "value4"s;
    const auto value5 = "value5"s;
    {
        auto&& testConveyor =
                conveyor<std::string>( [&](auto&& value) { results.push_back(std::move(value)); });

        testConveyor.push(value1);
        testConveyor.push(value2);
        testConveyor.push(value3);
        testConveyor.push(value4);
        testConveyor.push(value5);
    }

    EXPECT_THAT(results, ElementsAre(value1, value2, value3, value4, value5));
}

class NotMoveAssignable
{
public:
    explicit NotMoveAssignable(const std::string& value)
        : _value(value)
    {}

    NotMoveAssignable(NotMoveAssignable&&) = default;
    NotMoveAssignable& operator=(NotMoveAssignable&&) = delete;

    NotMoveAssignable(const NotMoveAssignable&) = default;
    NotMoveAssignable& operator=(const NotMoveAssignable&) = default;

    bool operator==(const NotMoveAssignable& other) const
    {
        return _value == other._value;
    }

private:
    std::string _value;
};

TEST(UnitTest_conveyor, notMoveAssignable)
{
    auto&& results = std::vector<NotMoveAssignable>();

    {
        auto&& testConveyor =
                conveyor<NotMoveAssignable>( [&](auto&& value) { results.push_back(std::move(value)); });

        testConveyor.push(NotMoveAssignable("value1"s));
        testConveyor.push(NotMoveAssignable("value2"s));
        testConveyor.push(NotMoveAssignable("value3"s));
        testConveyor.push(NotMoveAssignable("value4"s));
        testConveyor.push(NotMoveAssignable("value5"s));
    }

    EXPECT_THAT(results, ElementsAre(NotMoveAssignable("value1"s),
                                     NotMoveAssignable("value2"s),
                                     NotMoveAssignable("value3"s),
                                     NotMoveAssignable("value4"s),
                                     NotMoveAssignable("value5"s)));
}

class NotMoveConstructable
{
public:
    explicit NotMoveConstructable(const std::string& value)
            : _value(value)
    {}

    NotMoveConstructable(NotMoveConstructable&&) = delete;
    NotMoveConstructable& operator=(NotMoveConstructable&&) = default;

    NotMoveConstructable(const NotMoveConstructable&) = default;
    NotMoveConstructable& operator=(const NotMoveConstructable&) = default;

    explicit operator std::string() const
    {
            return _value;
    };

private:
    std::string _value;
};

TEST(UnitTest_conveyor, notMoveConstructable)
{
    auto&& results = std::vector<std::string>();

    {
        auto&& testConveyor =
                conveyor<std::unique_ptr<NotMoveConstructable> >( [&](auto&& value)
                                                                  { results.push_back(static_cast<std::string>(*value)); });

        testConveyor.push(std::make_unique<NotMoveConstructable>("value1"s));
        testConveyor.push(std::make_unique<NotMoveConstructable>("value2"s));
        testConveyor.push(std::make_unique<NotMoveConstructable>("value3"s));
        testConveyor.push(std::make_unique<NotMoveConstructable>("value4"s));
        testConveyor.push(std::make_unique<NotMoveConstructable>("value5"s));
    }

    EXPECT_THAT(results, ElementsAre("value1"s, "value2"s, "value3"s, "value4"s, "value5"s));
}

class NotCopyable
{
public:
    explicit NotCopyable(const std::string& value)
        : _value(value)
    {}

    NotCopyable(NotCopyable&&) = default;
    NotCopyable& operator=(NotCopyable&&) = default;

    NotCopyable(const NotCopyable&) = delete;
    NotCopyable& operator=(const NotCopyable&) = delete;

    explicit operator std::string() const
    {
        return _value;
    }

private:
    std::string _value;
};

TEST(UnitTest_conveyor, notCopyable)
{
    auto&& results = std::vector<std::string>();

    auto&& value2 = NotCopyable("value2"s);

    {
        auto&& testConveyor =
                conveyor<NotCopyable>( [&](auto&& value) { results.push_back(static_cast<std::string>(value)); });

        testConveyor.push(NotCopyable("value1"s));
        testConveyor.push(std::move(value2));
        testConveyor.push(NotCopyable("value3"s));
        testConveyor.push(NotCopyable("value4"s));
        testConveyor.push(NotCopyable("value5"s));
    }

    EXPECT_THAT(results, ElementsAre("value1"s, "value2"s, "value3"s, "value4"s, "value5"s));
}

TEST(UnitTest_conveyor, conveyorPipe)
{
    auto&& results = std::vector<std::string>();

    {
        auto&& outputConveyor =
                conveyor<std::string>( [&](auto&& value) { results.push_back(std::move(value)); });

        auto&& inputConveyor =
                conveyor<std::string>( [&](auto&& value) { outputConveyor.push(std::move(value)); });

        inputConveyor.push("value1"s);
        inputConveyor.push("value2"s);
        inputConveyor.push("value3"s);
        inputConveyor.push("value4"s);
        inputConveyor.push("value5"s);
    }

    EXPECT_THAT(results, ElementsAre("value1"s, "value2"s, "value3"s, "value4"s, "value5"s));
}

TEST(PerformanceTest_conveyor, move)
{
    auto&& pc = PerformanceCounter(*testing::UnitTest::GetInstance());
    auto&& results = std::vector<std::string>();

    auto&& stringValue = std::string(100, 'a');

    for (auto i = 0; i < 10; ++i)
    {
        {
            auto&& testConveyor =
                    conveyor<std::string>( [&](auto&& value) { results.push_back(std::move(value)); });

            pc.start();

            for (auto j = 0; j < 100000; ++j)
                testConveyor.push(stringValue + std::to_string(j));
        }

        pc.stop();
    }
}

TEST(PerformanceTest_conveyor, copy)
{
    auto&& pc = PerformanceCounter(*testing::UnitTest::GetInstance());

    auto&& results = std::vector<std::string>();
    auto&& stringValue = std::string(100, 'a');

    for (auto i = 0; i < 10; ++i)
    {
        {
            auto&& testConveyor =
                    conveyor<std::string>( [&](auto&& value) { results.push_back(std::move(value)); });

            pc.start();

            for (auto j = 0; j < 100000; ++j)
            {
                const auto value = stringValue + std::to_string(j);
                testConveyor.push(value);
            }
        }

        pc.stop();
    }
}