#include <benchmark/benchmark.h>

#include <random>

#include <boost/container/flat_set.hpp>

#include <container/flat_set.h>

class TestObject
{
public:
    explicit TestObject(int value) : value_(value) {}

    int getId() const { return value_; }

private:
    char payload_[100];
    int value_;
};

using TestObjectPtr = std::unique_ptr<TestObject>;

struct TestObjectLess
{
    bool operator()(const TestObject& l, const TestObject& r) const
    {
        return l.getId() < r.getId();
    }

    bool operator()(TestObject* l, TestObject* r) const
    {
        return l->getId() < r->getId();
    }

    bool operator()(const TestObjectPtr& l, const TestObjectPtr& r) const
    {
        return l->getId() < r->getId();
    }
};

template <typename Set>
auto constructSet(int size, const std::function<typename Set::value_type(int)>& creator)
{
    auto randomSet = Set();
    randomSet.reserve(static_cast<size_t>(size));

    for (auto i = 0; i < size; ++i)
        randomSet.insert(creator(i));

    return randomSet;
}

template <typename FlatSet>
static void flat_set_find_int(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();
        const auto creator = [](int value) { return value; };
        auto flatSet = constructSet<FlatSet>(size, creator);


        for (auto i = 0; i < size; ++i)
        {
            const auto value = rand() % size;

            state.ResumeTiming();

            benchmark::DoNotOptimize(flatSet.find(value));

            state.PauseTiming();
        }

    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_set_find_int, boost::container::flat_set<int>)->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);



template <typename FlatSet>
static void flat_set_find_ptr(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();
        const auto creator = [](int value) { return new TestObject(value); };
        auto flatSet = constructSet<FlatSet>(size, creator);


        for (auto i = 0; i < size; ++i)
        {
            const auto value = new TestObject(rand() % size);
            state.ResumeTiming();

            benchmark::DoNotOptimize(flatSet.find(value));

            state.PauseTiming();
        }

        for (auto toDelete : flatSet)
            delete toDelete;
    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_set_find_ptr, boost::container::flat_set<TestObject*, TestObjectLess>)
->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);



template <typename FlatSet>
static void flat_set_find_unique_ptr(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();
        const auto creator = [](int value) { return std::make_unique<TestObject>(value); };
        auto flatSet =
                constructSet<FlatSet>(size, creator);


        for (auto i = 0; i < size; ++i)
        {
            auto value = std::make_unique<TestObject>(rand() % size);
            state.ResumeTiming();

            benchmark::DoNotOptimize(flatSet.find(value));

            state.PauseTiming();
        }
    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_set_find_unique_ptr, boost::container::flat_set<TestObjectPtr, TestObjectLess>)
->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);


template <typename FlatSet>
static void flat_set_insert_int(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();

        auto flatSet = FlatSet();
        flatSet.reserve(static_cast<size_t>(size));

        for (auto i = 0; i < size; ++i)
        {
            const auto value = rand() % size;

            state.ResumeTiming();

            flatSet.insert(value);

            state.PauseTiming();
        }

    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_set_insert_int, boost::container::flat_set<int>)->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);


template <typename FlatSet>
static void flat_set_insert_ptr(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();

        auto flatSet = FlatSet();
        flatSet.reserve(static_cast<size_t>(size));

        for (auto i = 0; i < size; ++i)
        {
            const auto value = new TestObject(rand() % size);

            state.ResumeTiming();

            flatSet.insert(value);

            state.PauseTiming();
        }

        for (auto item : flatSet)
            delete item;

    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_set_insert_ptr, boost::container::flat_set<TestObject*, TestObjectLess>)
->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);


template <typename FlatSet>
static void flat_set_insert_unique_ptr(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();

        auto flatSet = FlatSet();
        flatSet.reserve(static_cast<size_t>(size));

        for (auto i = 0; i < size; ++i)
        {
            auto value = std::make_unique<TestObject>(rand() % size);

            state.ResumeTiming();

            flatSet.insert(std::move(value));

            state.PauseTiming();
        }

    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_set_insert_unique_ptr, boost::container::flat_set<TestObjectPtr, TestObjectLess>)
->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);


template <typename FlatSet>
static void flat_set_emplace_unique_ptr(benchmark::State& state)
{
    const auto size = state.range(0);

    for (auto _ : state)
    {
        state.PauseTiming();

        auto flatSet = FlatSet();
        flatSet.reserve(static_cast<size_t>(size));

        for (auto i = 0; i < size; ++i)
        {
            auto value = std::make_unique<TestObject>(rand() % size);

            state.ResumeTiming();

            flatSet.emplace(std::move(value));

            state.PauseTiming();
        }

    }
    state.SetItemsProcessed(state.iterations() * size);
    state.SetComplexityN(size);
}
BENCHMARK_TEMPLATE(flat_set_emplace_unique_ptr, boost::container::flat_set<TestObjectPtr, TestObjectLess>)
->RangeMultiplier(2)->Range(1 << 3, 1 << 12)->Complexity(benchmark::oN);