#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <container/flat_set.h>

class TestObject
{
public:
    explicit TestObject(short value) : _value(value) {}

    short getId() const { return _value; }

private:
    short _value;
};

TEST(PerformanceTest_boost_flat_map, insert)
{

}
