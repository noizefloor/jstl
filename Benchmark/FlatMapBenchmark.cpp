#include <benchmark/benchmark.h>

#include <random>

#include <boost/container/flat_map.hpp>

class TestObject
{
public:
    explicit TestObject(int value) : value_(value) {}

    int getId() const { return value_; }

private:
    char payload_[100];
    int value_;
};


template <typename Map, typename Creator>
auto constructMap(int size, const Creator& creator)
{
    auto randomSet = Map();
    randomSet.reserve(static_cast<size_t>(size));

    for (auto i = 0; i < size; ++i)
        randomSet.emplace(i, creator(i));

    return randomSet;
}


template <typename FlatMap>
static void flat_map_find_ptr(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();
        auto creator = [](int value) { return new TestObject(value); };
        auto flatMap = constructMap<FlatMap>(size, creator);


        for (auto i = 0; i < size; ++i)
        {
            const auto value = rand() % size;
            state.ResumeTiming();

            benchmark::DoNotOptimize(flatMap.find(value));

            state.PauseTiming();
        }

        for (auto toDelete : flatMap)
            delete toDelete.second;
    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_map_find_ptr, boost::container::flat_map<int, TestObject*>)
->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);


template <typename FlatMap>
static void flat_map_find_unique_ptr(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();
        auto flatMap = constructMap<FlatMap>(size, [](int value) { return std::make_unique<TestObject>(value); });


        for (auto i = 0; i < size; ++i)
        {
            const auto value = rand() % size;
            state.ResumeTiming();

            benchmark::DoNotOptimize(flatMap.find(value));

            state.PauseTiming();
        }
    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_map_find_unique_ptr, boost::container::flat_map<int, std::unique_ptr<TestObject> >)
->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);


template <typename FlatMap>
static void flat_map_insert_ptr(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();

        auto flatMap = FlatMap();
        flatMap.reserve(static_cast<size_t>(size));

        for (auto i = 0; i < size; ++i)
        {
            const auto value = new TestObject(rand() % size);

            state.ResumeTiming();

            flatMap.insert(std::make_pair(value->getId(), value));

            state.PauseTiming();
        }

        for (auto item : flatMap)
            delete item.second;

    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_map_insert_ptr, boost::container::flat_map<int, TestObject*>)
->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);


template <typename FlatMap>
static void flat_map_insert_unique_ptr(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();

        auto flatMap = FlatMap();
        flatMap.reserve(static_cast<size_t>(size));

        for (auto i = 0; i < size; ++i)
        {
            auto value = std::make_unique<TestObject>(rand() % size);

            state.ResumeTiming();

            flatMap.insert(std::make_pair(value->getId(), std::move(value)));

            state.PauseTiming();
        }
    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_map_insert_unique_ptr, boost::container::flat_map<int, std::unique_ptr<TestObject> >)
->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);



template <typename FlatMap>
static void flat_map_emplace_ptr(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();

        auto flatMap = FlatMap();
        flatMap.reserve(static_cast<size_t>(size));

        for (auto i = 0; i < size; ++i)
        {
            const auto value = new TestObject(rand() % size);

            state.ResumeTiming();

            flatMap.emplace(value->getId(), value);

            state.PauseTiming();
        }

        for (auto item : flatMap)
            delete item.second;

    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_map_emplace_ptr, boost::container::flat_map<int, TestObject*>)
->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);



template <typename FlatMap>
static void flat_map_emplace_unique_ptr(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();

        auto flatMap = FlatMap();
        flatMap.reserve(static_cast<size_t>(size));

        for (auto i = 0; i < size; ++i)
        {
            auto value = std::make_unique<TestObject>(rand() % size);

            state.ResumeTiming();

            flatMap.emplace(value->getId(), std::move(value));

            state.PauseTiming();
        }
    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_map_emplace_unique_ptr, boost::container::flat_map<int, std::unique_ptr<TestObject> >)
->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);
