#pragma once

#include <memory>

class TestValue
{
public:
    explicit TestValue(int value) : value_(value) {}

    int getId() const { return value_; }

private:
    char payload_[100];
    int value_;
};

using TestObjectPtr = std::unique_ptr<TestValue>;

struct TestObjectLess
{
    bool operator()(const TestValue& l, const TestValue& r) const
    {
        return l.getId() < r.getId();
    }

    bool operator()(TestValue* l, TestValue* r) const
    {
        return l->getId() < r->getId();
    }

    bool operator()(const TestObjectPtr& l, const TestObjectPtr& r) const
    {
        return l->getId() < r->getId();
    }
};


template <typename T>
struct ValueCreator
{
    T create(int value) const
    {
        return static_cast<T>(value);
    }

    void destroy() const {}
};

template <>
struct ValueCreator<TestValue*>
{
    ~ValueCreator()
    {
        destroy();
    }

    TestValue* create(int value) const
    {
        auto testObject = new TestValue(value);

        toDelete_.push_back(testObject);

        return testObject;
    }


    void destroy() const
    {
        for (auto toDelete : toDelete_)
            delete toDelete;

        toDelete_.clear();
    }

    mutable std::vector<TestValue*> toDelete_;
};

template <>
struct ValueCreator<TestObjectPtr>
{
    TestObjectPtr create(int value) const
    {
        return std::make_unique<TestValue>(value);
    }

    void destroy() const {}
};
