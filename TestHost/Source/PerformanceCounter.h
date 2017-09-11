#pragma once

#include <string>
#include <chrono>
#include <gtest/gtest.h>

class PerformanceCounter
{
public:
    PerformanceCounter(const std::string& testName, const std::string& testCaseName);
    PerformanceCounter(const testing::UnitTest& unitTest);
    ~PerformanceCounter();

    void start();
    void stop();

    void writeStatistics();

private:
    std::string toString(const std::chrono::steady_clock::duration& duration) const;

private:
    std::string _testName;
    std::string _testCaseName;

    bool _isStarted = false;
    std::chrono::steady_clock::time_point _startTime;

    std::vector<std::chrono::steady_clock::duration> _results;
};

