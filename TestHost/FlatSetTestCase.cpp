#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <container/flat_set.h>
#include <TestValue.h>

using jstd::flat_set;

using testing::ElementsAre;
using testing::SizeIs;
using testing::IsEmpty;

namespace
{
    TEST(UnitTest_flat_set, construct_initializer_list_unsorted)
    {
        auto&& testSet = flat_set<int>{ 10, 2, 5, 6, 3 };
    EXPECT_THAT(testSet, ElementsAre(2, 3, 5, 6, 10));
    }

    TEST(UnitTest_flat_set, construct_initializer_list_duplicates)
    {
        auto&& testSet = flat_set<int>{ 1, 1, 3, 3, 5, 5, 5, 8, 8 };
    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));
    }

    TEST(UnitTest_flat_set, construct_initializer_list_duplicates_unsorted)
    {
        auto&& testSet = flat_set<int>{ 1, 3, 5, 3, 5, 1, 5, 8, 5, 1, 8 };
    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));
    }

    TEST(UnitTest_flat_set, construct_from_iterator)
    {
        auto source = std::vector<int>{ 1, 3, 5, 3, 5, 1, 5, 8, 5, 1, 8 };
        auto&& testSet = flat_set<int>(source.cbegin(), source.cend());
    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));
    }

    TEST(UnitTest_flat_set, copy_constructor)
    {
        auto&& source = flat_set<int>{ 1, 3, 5, 3, 5, 1, 5, 8, 5, 1, 8 };
        auto&& testSet = flat_set<int>(source);
    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));

    EXPECT_EQ(4, testSet.capacity());
    }

    TEST(UnitTest_flat_set, move_constructor)
    {
        auto&& source = flat_set<int>{ 1, 3, 5, 3, 5, 1, 5, 8, 5, 1, 8 };
        auto&& testSet = flat_set<int>(std::move(source));
    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));

    EXPECT_EQ(4, testSet.capacity());
    }

    TEST(UnitTest_flat_set, copy_operator)
    {
        auto&& source = flat_set<int>{ 1, 3, 5, 3, 5, 1, 5, 8, 5, 1, 8 };
        auto&& testSet = flat_set<int>();

        testSet = source;
    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));

    EXPECT_EQ(4, testSet.capacity());
    }

    TEST(UnitTest_flat_set, move_operator)
    {
        auto&& source = flat_set<int>{ 1, 3, 5, 3, 5, 1, 5, 8, 5, 1, 8 };
        auto&& testSet = flat_set<int>();

        testSet = std::move(source);
    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));

    EXPECT_EQ(4, testSet.capacity());
    }

    TEST(UnitTest_flat_set, initializer_list_operator)
    {
        auto&& testSet = flat_set<int>();

        testSet = { 1, 3, 5, 3, 5, 1, 5, 8, 5, 1, 8 };
    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));
    }

    TEST(UnitTest_flat_set, empty_true)
    {
        auto&& testSet = flat_set<int>();
    EXPECT_TRUE(testSet.empty());
    }

    TEST(UnitTest_flat_set, empty_false)
    {
        auto&& testSet = flat_set<int>{ 1, 2, 3 };
    EXPECT_FALSE(testSet.empty());
    }

    TEST(UnitTest_flat_set, size_zero)
    {
        auto&& testSet = flat_set<int>();
    EXPECT_EQ(0, testSet.size());
    }

    TEST(UnitTest_flat_set, size)
    {
        auto&& testSet = flat_set<int>{ 1, 2, 3 };
    EXPECT_EQ(3, testSet.size());
    }

    TEST(UnitTest_flat_set, max_size)
    {
        auto testSet = flat_set<int>();
        auto&& testVector = flat_set<int>::sequence_type();

    EXPECT_EQ(testVector.max_size(), testSet.max_size());
    }

    TEST(UnitTest_flat_set, insert_lvalue_ref)
    {
        auto source = std::vector<TestValue>{ TestValue(1), TestValue(3), TestValue(5), TestValue(8) };
        auto&& testSet = flat_set<TestValue, TestValueLess>();

        for (const auto& item : source)
        {
            auto result = testSet.insert(item);
        EXPECT_TRUE(result.second);
        EXPECT_EQ(item.getId(), result.first->getId());
        }

        const auto testValue = TestValue(5);
        auto result = testSet.insert(testValue);

    EXPECT_FALSE(result.second);
    EXPECT_EQ(5, result.first->getId());

        auto actualValues = std::vector<int>();
        for (const auto& item : source)
            actualValues.push_back(item.getId());

    EXPECT_THAT(actualValues, ElementsAre(1, 3, 5, 8));
    }

    TEST(UnitTest_flat_set, insert_from_iterator)
    {
        auto source = std::vector<int>{ 1, 3, 5, 3, 5, 1, 5, 8, 5, 1, 8 };
        auto&& testSet = flat_set<int>();

        testSet.insert(source.cbegin(), source.cend());

    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));
    }

    TEST(UnitTest_flat_set, insert_from_initializer_list)
    {
        auto&& testSet = flat_set<int>();

        testSet.insert({ 1, 3, 5, 3, 5, 1, 5, 8, 5, 1, 8 });

    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));
    }

    template <typename T>
    class UnitTest_flat_set : public testing::Test
    {
    public:
        using FlatSetType = T;
        using ValueType = typename FlatSetType::value_type;


        FlatSetType createFlatSet(const std::initializer_list<int>& initializerList) const
        {
            auto&& flatSet = FlatSetType();
            for (const auto item : initializerList)
                flatSet.insert(valueTool_.create(item));

            return std::move(flatSet);
        }

        std::vector<int> getInnerValues(const FlatSetType& flatSet) const
        {
            auto&& result = std::vector<int>();
            for (const auto& item : flatSet)
                result.push_back(valueTool_.getInt(item));

            return std::move(result);
        }

        TestValueTool<ValueType> valueTool_;
    };

    using FlatSetTypes = ::testing::Types<
        flat_set<short>,
        flat_set<int>,
        flat_set<long long>,
        flat_set<TestValue*, TestValueLess>,
        flat_set<TestValuePtr, TestValueLess>
    >;

    TYPED_TEST_CASE(UnitTest_flat_set, FlatSetTypes);



    TYPED_TEST(UnitTest_flat_set, forward_iterator)
    {
        auto&& testSet = this->createFlatSet({ 1, 2, 3 ,4, 5, 6, 7, 8, 9});

        auto expectedValue = 1;
        for (auto it = testSet.begin(); it != testSet.end(); ++it)
        EXPECT_EQ(expectedValue++, this->valueTool_.getInt(*it));
    }

    TYPED_TEST(UnitTest_flat_set, forward_const_iterator)
    {
        auto&& testSet = this->createFlatSet({ 1, 2, 3 ,4, 5, 6, 7, 8, 9});

        auto expectedValue = 1;
        for (auto it = testSet.cbegin(); it != testSet.cend(); ++it)
        EXPECT_EQ(expectedValue++, this->valueTool_.getInt(*it));
    }

    TYPED_TEST(UnitTest_flat_set, reverse_iterator)
    {
        auto&& testSet = this->createFlatSet({ 1, 2, 3 ,4, 5, 6, 7, 8, 9});

        auto expectedValue = 9;
        for (auto it = testSet.rbegin(); it != testSet.rend(); ++it)
        EXPECT_EQ(expectedValue--, this->valueTool_.getInt(*it));
    }

    TYPED_TEST(UnitTest_flat_set, reverse_const_iterator)
    {
        auto&& testSet = this->createFlatSet({ 1, 2, 3 ,4, 5, 6, 7, 8, 9});

        auto expectedValue = 9;
        for (auto it = testSet.crbegin(); it != testSet.crend(); ++it)
        EXPECT_EQ(expectedValue--, this->valueTool_.getInt(*it));
    }

    TYPED_TEST(UnitTest_flat_set, capacity_reserve)
    {
        auto testSet = TypeParam();

    ASSERT_EQ(0, testSet.capacity());

        testSet.reserve(100);

    EXPECT_EQ(100, testSet.capacity());
    }

    TYPED_TEST(UnitTest_flat_set, shrink_to_fit)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        testSet.reserve(100);
    ASSERT_EQ(100, testSet.capacity());

        testSet.shrink_to_fit();

    EXPECT_EQ(5, testSet.capacity());
    }


    TYPED_TEST(UnitTest_flat_set, clear)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

    ASSERT_THAT(testSet, SizeIs(5));

        testSet.clear();

    EXPECT_THAT(testSet, IsEmpty());
    }

    TYPED_TEST(UnitTest_flat_set, insert_not_existing_begin)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        auto result = testSet.insert(this->valueTool_.create(-1));

    EXPECT_TRUE(result.second);
    EXPECT_EQ(-1, this->valueTool_.getInt(*result.first));

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(-1, 2, 3, 4, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, insert_not_existing_mid)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        auto result = testSet.insert(this->valueTool_.create(5));

    EXPECT_TRUE(result.second);
    EXPECT_EQ(5, this->valueTool_.getInt(*result.first));

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3, 4, 5, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, insert_not_existing_end)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        auto result = testSet.insert(this->valueTool_.create(50));

    EXPECT_TRUE(result.second);
    EXPECT_EQ(50, this->valueTool_.getInt(*result.first));

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3, 4, 7, 9, 50));
    }

    TYPED_TEST(UnitTest_flat_set, insert_existing_begin)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        auto result = testSet.insert(this->valueTool_.create(2));

    EXPECT_FALSE(result.second);
    EXPECT_EQ(2, this->valueTool_.getInt(*result.first));

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3, 4, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, insert_existing_mid)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        auto result = testSet.insert(this->valueTool_.create(4));

    EXPECT_FALSE(result.second);
    EXPECT_EQ(4, this->valueTool_.getInt(*result.first));

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3, 4, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, insert_existing_end)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        auto result = testSet.insert(this->valueTool_.create(9));

    EXPECT_FALSE(result.second);
    EXPECT_EQ(9, this->valueTool_.getInt(*result.first));

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3, 4, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_const_iterator_first)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        testSet.erase(testSet.cbegin());

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(3, 4, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_const_iterator_mid)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        testSet.erase(testSet.cbegin() + 3);

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3, 4, 9));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_const_iterator_last)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        testSet.erase(testSet.cend() - 1);

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3, 4, 7));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_iterator_first)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        testSet.erase(testSet.begin());

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(3, 4, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_iterator_mid)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        testSet.erase(testSet.begin() + 3);

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3, 4, 9));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_iterator_last)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        testSet.erase(testSet.end() - 1);

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3, 4, 7));
    }


    TYPED_TEST(UnitTest_flat_set, erase_by_range_begin)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        testSet.erase(testSet.cbegin(), testSet.cbegin() + 2);

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(4, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_range_mid)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        testSet.erase(testSet.cbegin() + 1, testSet.cend() - 1);

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 9));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_range_end)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        testSet.erase(testSet.cbegin() + 2, testSet.cend());

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_key_existing)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

    EXPECT_EQ(1, testSet.erase(this->valueTool_.create(3)));

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 4, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_key_not_existing)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

    EXPECT_EQ(0, testSet.erase(this->valueTool_.create(50)));

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3, 4, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_comparable_existing)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

    EXPECT_EQ(1, testSet.erase(3));

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 4, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, erase_by_compareable_not_existing)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

    EXPECT_EQ(0, testSet.erase(50));

    EXPECT_THAT(this->getInnerValues(testSet), ElementsAre(2, 3, 4, 7, 9));
    }

    TYPED_TEST(UnitTest_flat_set, count_by_key_existing)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

    EXPECT_EQ(1, testSet.count(this->valueTool_.create(3)));
    }

    TYPED_TEST(UnitTest_flat_set, count_by_key_not_existing)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

    EXPECT_EQ(0, testSet.count(this->valueTool_.create(50)));
    }

    TYPED_TEST(UnitTest_flat_set, count_by_comparable_existing)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

    EXPECT_EQ(1, testSet.count(3));
    }

    TYPED_TEST(UnitTest_flat_set, count_by_compareable_not_existing)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

    EXPECT_EQ(0, testSet.count(50));
    }

    TYPED_TEST(UnitTest_flat_set, find_by_key_existing)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

        for (const auto& item : testSet)
        {
            auto result = testSet.find(item);
        EXPECT_NE(testSet.cend(), result);
        EXPECT_EQ(this->valueTool_.getInt(item), this->valueTool_.getInt(*result));
        }
    }

    TYPED_TEST(UnitTest_flat_set, find_by_key_not_existing)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

    EXPECT_EQ(testSet.cend(), testSet.find(this->valueTool_.create(50)));
    }

    TYPED_TEST(UnitTest_flat_set, find_by_comparable_existing)
    {
        auto values = { 4, 7, 3, 2, 9 };
        auto testSet = this->createFlatSet(values);

        for (auto value : values)
        {
            auto result = testSet.find(value);
        EXPECT_NE(testSet.cend(), result);
        EXPECT_EQ(value, this->valueTool_.getInt(*result));
        }
    }

    TYPED_TEST(UnitTest_flat_set, find_by_compareable_not_existing)
    {
        auto testSet = this->createFlatSet({ 4, 7, 3, 2, 9 });

    EXPECT_EQ(testSet.cend(), testSet.find(50));
    }
}
