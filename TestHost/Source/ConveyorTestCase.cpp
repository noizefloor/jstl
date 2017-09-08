#include <string>
#include <vector>
#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <concurrency/conveyor.h>

using jstd::conveyor;
using testing::ElementsAre;
using namespace std::literals::string_literals;

TEST(UnitTest_conveyor, pushAndWait)
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