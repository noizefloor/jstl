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

using TestValuePtr = std::unique_ptr<TestValue>;

struct TestValueLess
{
    bool operator()(const TestValue& l, const TestValue& r) const
    {
        return l.getId() < r.getId();
    }

    bool operator()(const TestValue& l, int r) const
    {
        return l.getId() < r;
    }

    bool operator()(int l, const TestValue& r) const
    {
        return l < r.getId();
    }

    bool operator()(TestValue* l, TestValue* r) const
    {
        return l->getId() < r->getId();
    }

    bool operator()(TestValue* l, int r) const
    {
        return l->getId() < r;
    }

    bool operator()(int l, TestValue* r) const
    {
        return l < r->getId();
    }

    bool operator()(const TestValuePtr& l, const TestValuePtr& r) const
    {
        return l->getId() < r->getId();
    }

    bool operator()(const TestValuePtr& l, int r) const
    {
        return l->getId() < r;
    }

    bool operator()(int l, const TestValuePtr& r) const
    {
        return l < r->getId();
    }
};

struct TestValueEqual
{
    bool operator()(const TestValue& l, const TestValue& r) const
    {
        return l.getId() == r.getId();
    }

    bool operator()(const TestValue& l, int r) const
    {
        return l.getId() == r;
    }

    bool operator()(int l, const TestValue& r) const
    {
        return l == r.getId();
    }

    bool operator()(TestValue* l, TestValue* r) const
    {
        return l->getId() == r->getId();
    }

    bool operator()(TestValue* l, int r) const
    {
        return l->getId() == r;
    }

    bool operator()(int l, TestValue* r) const
    {
        return l == r->getId();
    }

    bool operator()(const TestValuePtr& l, const TestValuePtr& r) const
    {
        return l->getId() == r->getId();
    }

    bool operator()(const TestValuePtr& l, int r) const
    {
        return l->getId() == r;
    }

    bool operator()(int l, const TestValuePtr& r) const
    {
        return l == r->getId();
    }
};

struct TestValueHash
{
    size_t operator()(const TestValue& value) const
    {
        return std::hash<int>()(value.getId());
    }

    size_t operator()(const TestValue* value) const
    {
        return std::hash<int>()(value->getId());
    }

    size_t operator()(const TestValuePtr& value) const
    {
        return std::hash<int>()(value->getId());
    }
};

template <typename T>
struct TestValueTool
{
    T create(int value) const
    {
        return static_cast<T>(value);
    }

    int getInt(T value) const
    {
        return static_cast<int>(value);
    }

    void destroy() const {}
};

template <>
struct TestValueTool<TestValue*>
{
    ~TestValueTool()
    {
        destroy();
    }

    TestValue* create(int value) const
    {
        auto testObject = new TestValue(value);

        toDelete_.push_back(testObject);

        return testObject;
    }

    int getInt(const TestValue* value) const
    {
        return value->getId();
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
struct TestValueTool<TestValuePtr>
{
    TestValuePtr create(int value) const
    {
        return std::make_unique<TestValue>(value);
    }

    int getInt(const TestValuePtr& value) const
    {
        return value->getId();
    }

    void destroy() const {}
};
