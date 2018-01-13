#include <chrono>
using namespace std::chrono;

#include <benchmark/benchmark.h>

#include <concurrency/conveyor_function.h>

static void conveyor_function_move(benchmark::State& state)
{

    auto&& stringValue = std::string(100, 'a');

    for (auto _ : state)
    {
        state.PauseTiming();
        auto&& results = std::vector<std::string>();

        state.ResumeTiming();
        jstd::conveyor_function([&](jstd::conveyor_forwarder<std::string>& f)
                                {
                                    for (auto j = 0; j < state.range(0); ++j)
                                        f.push(stringValue + std::to_string(j));
                                }, [&](std::string&& value) { results.push_back(std::move(value)); });
        state.PauseTiming();

    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}

BENCHMARK(conveyor_function_move)->RangeMultiplier(2)->Range(8, 8<<4)->UseRealTime();

template <bool parallel>
static void conveyor_function(benchmark::State& state)
{
    for (auto _ : state)
    {
        const auto wait = milliseconds(state.range(1));
        state.PauseTiming();

        if (parallel)
        {
            state.ResumeTiming();
            jstd::conveyor_function([&](jstd::conveyor_forwarder<milliseconds>& f)
                                    {
                                        for (auto j = 0; j < state.range(0); ++j)
                                        {
                                            std::this_thread::sleep_for(wait);
                                            f.push(milliseconds(wait));
                                        }
                                    }, [&](milliseconds&& value)
                                    {
                                        std::this_thread::sleep_for(value);
                                    });
        }
        else
        {
            state.ResumeTiming();
            for (auto j = 0; j < state.range(0); ++j)
                std::this_thread::sleep_for(wait * 2);
        }
        state.PauseTiming();

    }
    state.SetItemsProcessed(state.iterations() * state.range(0));
}

BENCHMARK_TEMPLATE(conveyor_function, false)
->RangeMultiplier(2)->Ranges({{1 << 3, 1 << 6}, {1, 1 << 6}})->UseRealTime();

BENCHMARK_TEMPLATE(conveyor_function, true)
->RangeMultiplier(2)->Ranges({{1 << 3, 1 << 6}, {1, 1 << 6}})->UseRealTime();