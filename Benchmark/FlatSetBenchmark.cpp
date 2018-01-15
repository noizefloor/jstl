#include <benchmark/benchmark.h>

#include <boost/container/flat_set.hpp>

#include <container/flat_set.h>

#include "TestValue.h"

template <typename FlatSet>
static void flat_set_find(benchmark::State& state)
{
    const auto size = state.range(0);

    const auto valueTool = TestValueTool<typename FlatSet::value_type>();

    auto flatSet = FlatSet();
    flatSet.reserve(static_cast<size_t>(size));

    for (auto j = 0; j < size; ++j)
        flatSet.insert(valueTool.create(j));

    auto testValueIt = flatSet.cbegin();

    for (auto _ : state)
    {
        auto&& testItem = *testValueIt++;
        benchmark::DoNotOptimize(flatSet.find(testItem));

        if (testValueIt == flatSet.cend())
            testValueIt = flatSet.cbegin();
    }

    state.SetItemsProcessed(state.iterations());
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_set_find, boost::container::flat_set<short>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_find, jstd::flat_set<short>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_find, boost::container::flat_set<int>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_find, jstd::flat_set<int>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_find, boost::container::flat_set<long long>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_find, jstd::flat_set<long long>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_find, boost::container::flat_set<TestValue*, TestValueLess>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_find, jstd::flat_set<TestValue*, TestValueLess>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_find, boost::container::flat_set<std::unique_ptr<TestValue>, TestValueLess>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_find, jstd::flat_set<std::unique_ptr<TestValue>, TestValueLess>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);


template <typename FlatSet, bool reserved>
static void flat_set_insert(benchmark::State& state)
{
    const auto doubleSize = state.range(0) * 2;
    const auto size = static_cast<size_t>(state.range(0));

    const auto valueTool = TestValueTool<typename FlatSet::value_type>();

    auto flatSetCreator = [&]
    {
        auto&& flatSet = FlatSet();

        if (reserved)
            flatSet.reserve(size * 2);
        else
            flatSet.reserve(size);

        for (auto j = 0; j < doubleSize; j += 2)
            flatSet.insert(valueTool.create(j));

        return std::move(flatSet);
    };

    auto testValuesCreator = [&]
    {
        auto values = std::vector<typename FlatSet::value_type>();

        for (auto j = 1; j < doubleSize; j += 2)
            values.push_back(valueTool.create(j));

        return std::move(values);
    };

    auto&& flatSet = flatSetCreator();
    auto&& values = testValuesCreator();

    auto value = values.begin();

    for (auto _ : state)
    {
        flatSet.insert(std::move(*value++));

        if (value == values.end())
        {
            state.PauseTiming();
            valueTool.destroy();
            flatSet = flatSetCreator();
            values = testValuesCreator();

            value = values.begin();
            state.ResumeTiming();
        }
    }

    state.SetItemsProcessed(state.iterations());
    state.SetComplexityN(state.range(0));
}

BENCHMARK_TEMPLATE(flat_set_insert, boost::container::flat_set<short>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_insert, jstd::flat_set<short>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_insert, boost::container::flat_set<int>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_insert, jstd::flat_set<int>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_insert, boost::container::flat_set<long long>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_insert, jstd::flat_set<long long>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_insert, boost::container::flat_set<TestValue*, TestValueLess>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_insert, jstd::flat_set<TestValue*, TestValueLess>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_insert, boost::container::flat_set<std::unique_ptr<TestValue>, TestValueLess>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_set_insert, jstd::flat_set<std::unique_ptr<TestValue>, TestValueLess>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);


