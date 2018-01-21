#include <benchmark/benchmark.h>

#include <random>
#include <unordered_map>

#include <boost/container/flat_map.hpp>

#include "TestValue.h"


template <typename FlatMap>
static void flat_map_find(benchmark::State& state)
{
    const auto size = state.range(0);

    const auto valueTool = TestValueTool<typename FlatMap::mapped_type>();

    auto flatMap = FlatMap();
    flatMap.reserve(static_cast<size_t>(size));

    for (auto j = 0; j < size; ++j)
        flatMap.insert(std::make_pair(j, valueTool.create(j)));

    auto testValue = 0;

    for (auto _ : state)
    {
        benchmark::DoNotOptimize(flatMap.find(testValue++));

        if (testValue == size)
            testValue = 0;
    }

    state.SetItemsProcessed(state.iterations());
    state.SetComplexityN(size);
}


BENCHMARK_TEMPLATE(flat_map_find, boost::container::flat_map<int, TestValue*>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_map_find, boost::container::flat_map<int, std::unique_ptr<TestValue> >)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_map_find, std::unordered_map<int, TestValue*>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_map_find, std::unordered_map<int, std::unique_ptr<TestValue>>)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);


template <typename FlatMap, bool reserved>
static void flat_map_insert(benchmark::State& state)
{
    const auto doubleSize = state.range(0) * 2;
    const auto size = static_cast<size_t>(state.range(0));

    const auto valueTool = TestValueTool<typename FlatMap::mapped_type>();

    auto flatMapCreator = [&]
    {
        auto&& flatMap = FlatMap();

        if (reserved)
            flatMap.reserve(size * 2);
        else
            flatMap.reserve(size);

        for (auto j = 0; j < doubleSize; j += 2)
            flatMap.emplace(j, valueTool.create(j));

        return std::move(flatMap);
    };

    auto testValuesCreator = [&]
    {
        auto values = std::vector<typename FlatMap::value_type>();

        for (auto j = 1; j < doubleSize; j += 2)
            values.emplace_back(j, valueTool.create(j));

        return std::move(values);
    };

    auto&& flatMap = flatMapCreator();
    auto&& values = testValuesCreator();

    auto value = values.begin();

    for (auto _ : state)
    {
        flatMap.insert(std::move(*value++));

        if (value == values.end())
        {
            state.PauseTiming();
            valueTool.destroy();
            flatMap = flatMapCreator();
            values = testValuesCreator();

            value = values.begin();
            state.ResumeTiming();
        }
    }

    state.SetItemsProcessed(state.iterations());
    state.SetComplexityN(state.range(0));
}

BENCHMARK_TEMPLATE(flat_map_insert, boost::container::flat_map<int, TestValue*>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_map_insert, boost::container::flat_map<int, std::unique_ptr<TestValue> >, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_map_insert, std::unordered_map<int, TestValue*>, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);

BENCHMARK_TEMPLATE(flat_map_insert, std::unordered_map<int, std::unique_ptr<TestValue> >, true)
->RangeMultiplier(2)->Range(1 << 7, 1 << 13)->Complexity(benchmark::oN);
