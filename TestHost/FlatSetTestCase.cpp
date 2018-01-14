#include <gtest/gtest.h>
#include <gmock/gmock.h>

#include <container/flat_set.h>

using jstd::flat_set;

using testing::ElementsAre;

TEST(flat_set, empty_true)
{
    auto&& testSet = flat_set<int>();
    EXPECT_TRUE(testSet.empty());
}

TEST(flat_set, empty_false)
{
    auto&& testSet = flat_set<int>({ 1, 2, 3 });
    EXPECT_FALSE(testSet.empty());
}

TEST(flat_set, size_zero)
{
    auto&& testSet = flat_set<int>();
    EXPECT_EQ(0, testSet.size());
}

TEST(flat_set, size)
{
    auto&& testSet = flat_set<int>({ 1, 2, 3 });
    EXPECT_EQ(3, testSet.size());
}

TEST(flat_set, construct_initializer_list_unsorted)
{
    auto&& testSet = flat_set<int>({ 10, 2, 5, 6, 3 });
    EXPECT_THAT(testSet, ElementsAre(2, 3, 5, 6, 10));
}

TEST(flat_set, construct_initializer_list_duplicates)
{
    auto&& testSet = flat_set<int>({ 1, 1, 3, 3, 5, 5, 5, 8, 8 });
    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));
}

TEST(flat_set, construct_initializer_list_duplicates_unsorted)
{
    auto&& testSet = flat_set<int>({ 1, 3, 5, 3, 5, 1, 5, 8, 5, 1, 8 });
    EXPECT_THAT(testSet, ElementsAre(1, 3, 5, 8));
}

