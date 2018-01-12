#include <benchmark/benchmark.h>

#include <concurrency/conveyor_function.h>

static void conveyor_function_move(benchmark::State& state)
{
    auto&& results = std::vector<std::string>();

    auto&& stringValue = std::string(100, 'a');

    for (auto _ : state)
    {
        jstd::conveyor_function([&](jstd::conveyor_forwarder<std::string>& f)
                                {
                                    for (auto j = 0; j < state.range(0); ++j)
                                        f.push(stringValue + std::to_string(j));
                                }, [&](std::string&& value) { results.push_back(std::move(value)); });
    }
}

BENCHMARK(conveyor_function_move)->RangeMultiplier(2)->Range(8, 8<<4);