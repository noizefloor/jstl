#include <benchmark/benchmark.h>

#include <concurrency/conveyor.h>

static void conveyor_class_move(benchmark::State& state)
{
    auto&& results = std::vector<std::string>();

    auto&& stringValue = std::string(100, 'a');

    for (auto _ : state)
    {
        auto&& testConveyor =
                jstd::conveyor<std::string>( [&](auto&& value) { results.push_back(std::move(value)); });

        for (auto j = 0; j < state.range(0); ++j)
            testConveyor.push(stringValue + std::to_string(j));
    }
}

BENCHMARK(conveyor_class_move)->RangeMultiplier(2)->Range(8, 8<<4);


static void conveyor_class_copy(benchmark::State& state)
{
    auto&& results = std::vector<std::string>();
    auto&& stringValue = std::string(100, 'a');

    for (auto _ : state)
    {
        auto&& testConveyor =
                jstd::conveyor<std::string>( [&](auto&& value) { results.push_back(std::move(value)); });

        for (auto j = 0; j < state.range(0); ++j)
        {
            const auto value = stringValue + std::to_string(j);
            testConveyor.push(value);
        }
    }
}

BENCHMARK(conveyor_class_copy)->RangeMultiplier(2)->Range(8, 8<<4);
