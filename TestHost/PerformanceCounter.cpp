#include "stdafx.h"

#include "PerformanceCounter.h"


using namespace std::literals::string_literals;

PerformanceCounter::PerformanceCounter(const std::string& testName, const std::string& testCaseName)
    : _testName(testName)
    , _testCaseName(testCaseName)
{

}

PerformanceCounter::PerformanceCounter(const testing::UnitTest& unitTest)
    : PerformanceCounter(unitTest.current_test_info()->name(), unitTest.current_test_info()->test_case_name())
{

}

PerformanceCounter::~PerformanceCounter()
{
    try
    {
        writeStatistics();
    }
    catch (...)
    {

    }
}

void PerformanceCounter::start()
{
    _isStarted = true;
    _startTime = std::chrono::steady_clock::now();
}

void PerformanceCounter::stop()
{
    if (!_isStarted)
        return;

    auto&& stopTime = std::chrono::steady_clock::now();
    _isStarted = false;

    _results.push_back(stopTime - _startTime);
}

namespace
{
    std::tm localtime_safe(const std::time_t& time)
    {
        auto&& tm_snapshot = std::tm();
#if (defined(WIN32) || defined(_WIN32) || defined(__WIN32__))
        localtime_s(&tm_snapshot, &time);
#else
        localtime_r(&time, &tm_snapshot);
#endif
        return tm_snapshot;
    }
}

void PerformanceCounter::writeStatistics()
{
    if (_results.empty())
        return;

    auto&& output = std::ofstream("Statistics_" + _testCaseName + "." + _testName + ".log", std::ios::app);

    auto&& total = std::accumulate(_results.cbegin(), _results.cend(), std::chrono::steady_clock::duration());
    auto&& minDuration = std::min_element(_results.cbegin(), _results.cend());
    auto&& maxDuration = std::max_element(_results.cbegin(), _results.cend());

    auto&& tm = localtime_safe(std::time(nullptr));
    output << std::put_time(&tm, "%c %Z")
           << " - times: " << _results.size()
           << ", total: " << toString(total)
           << ", average: " << toString(total / _results.size())
           << ", minimum: " << toString(*minDuration)
           << ", maximum: " << toString(*maxDuration)
           << std::endl;
}

namespace
{
    template <typename First, typename Second>
    std::string toStringInternal(const std::chrono::steady_clock::duration& duration,
                         const std::string& firstUnit, const std::string& secondUnit)
    {
        auto&& firstDuration = std::chrono::duration_cast<First>(duration);

        auto&& result = std::to_string(firstDuration.count()) + firstUnit;

        if (firstDuration.count() < 10)
        {
            auto&& secondDuration =
                    std::chrono::duration_cast<Second>(duration) - std::chrono::duration_cast<Second>(firstDuration);
            result += " "s + std::to_string(secondDuration.count()) + secondUnit;
        }

        return result;
    };
}

std::string PerformanceCounter::toString(const std::chrono::steady_clock::duration& duration) const
{
    if (std::chrono::duration_cast<std::chrono::hours>(duration).count() > 0)
        return toStringInternal<std::chrono::hours, std::chrono::minutes>(duration, "h"s, "m"s);
    else if (std::chrono::duration_cast<std::chrono::minutes>(duration).count() > 0)
        return toStringInternal<std::chrono::minutes, std::chrono::seconds>(duration, "m"s, "s"s);
    else if (std::chrono::duration_cast<std::chrono::seconds>(duration).count() > 0)
        return toStringInternal<std::chrono::seconds, std::chrono::milliseconds>(duration, "s"s, "ms"s);
    else if (std::chrono::duration_cast<std::chrono::milliseconds>(duration).count() > 0)
        return toStringInternal<std::chrono::milliseconds, std::chrono::microseconds>(duration, "ms"s, "µs"s);
    else if (std::chrono::duration_cast<std::chrono::microseconds>(duration).count() > 0)
        return toStringInternal<std::chrono::microseconds, std::chrono::nanoseconds>(duration, "µs"s, "ns"s);
    return std::to_string(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()) + "ns"s;
}
