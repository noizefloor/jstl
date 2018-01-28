#include <string>
#include <set>
#include <unordered_set>
#include <map>
#include <unordered_map>

using namespace std::string_literals;

#include <gperftools/heap-profiler.h>

#include <boost/container/flat_set.hpp>
#include <boost/container/flat_map.hpp>
#include <container/flat_set.h>
#include <container/flat_hash_map.h>

namespace
{
    template<typename T>
    void profileSet(const std::string& typeName, int size)
    {
        const auto prefix = "MemoryProfile_"s + typeName;
        HeapProfilerStart(prefix.c_str());

        auto container = T();

        for (auto i = 0; i < size; ++i)
            container.insert(i);

        HeapProfilerDump("");
        HeapProfilerStop();
    }

    template<typename T>
    void profileMap(const std::string& typeName, int size)
    {
        const auto prefix = "MemoryProfile_"s + typeName;
        HeapProfilerStart(prefix.c_str());

        auto container = T();

        for (auto i = 0; i < size; ++i)
            container.emplace(i, i);

        HeapProfilerDump("");
        HeapProfilerStop();
    }

} // namespace

int main()
{
    auto size = 1 << 22;

    profileSet<jstd::flat_set<int> >("jstd__flat_set", size);
    profileSet<boost::container::flat_set<int> >("boost__flat_set", size);
    profileSet<std::set<int> >("std__set", size);
    profileSet<std::unordered_set<int> >("std__unordered_set", size);

    profileMap<jstd::flat_hash_map<int, int> >("jstd__flat_hash_map", size);
    profileMap<boost::container::flat_map<int, int> >("boost__flat_map", size);
    profileMap<std::map<int, int> >("std__map", size);
    profileMap<std::unordered_map<int, int> >("std__unordered_map", size);

    return 0;
}