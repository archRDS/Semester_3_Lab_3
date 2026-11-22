#include "array.h"
#include "avl_tree.h"
#include "double_list.h"
#include "hash_table.h"
#include "queue.h"
#include "single_list.h"
#include "stack.h"

#include <gtest/gtest.h>

#include <stdexcept>
#include <string>
#include <vector>

using std::string;
using std::vector;

TEST(ArrayTest, GrowsAndShiftsElements) {
    Array array;
    EXPECT_EQ(array.size(), 0);
    EXPECT_TRUE(array.read().empty());
    for (int i = 0; i < 9; ++i) array.push(std::to_string(i));
    array.insert(0, "first");
    array.insert(3, "middle");
    array.set(2, "changed");
    EXPECT_EQ(array.get(2), "changed");
    EXPECT_EQ(array.erase(3), "middle");
    EXPECT_EQ(array.erase(array.size() - 1), "8");
    EXPECT_EQ(array.read(), (vector<string>{"first", "0", "changed", "2", "3", "4", "5", "6", "7"}));
    EXPECT_EQ(array.size(), 9);
}

TEST(ArrayTest, RejectsInvalidIndices) {
    Array array;
    EXPECT_THROW(array.get(0), std::runtime_error);
    EXPECT_THROW(array.insert(-1, "x"), std::runtime_error);
    EXPECT_THROW(array.insert(1, "x"), std::runtime_error);
    array.push("value");
    EXPECT_THROW(array.set(-1, "x"), std::runtime_error);
    EXPECT_THROW(array.set(1, "x"), std::runtime_error);
    EXPECT_THROW(array.erase(1), std::runtime_error);
    EXPECT_EQ(array.get(0), "value");
}

template <class List>
class ListTest : public testing::Test {};

using ListTypes = testing::Types<SingleList, DoubleList>;
TYPED_TEST_SUITE(ListTest, ListTypes);

TYPED_TEST(ListTest, InsertsAndRemovesAtBothEndsAndInTheMiddle) {
    TypeParam list;
    EXPECT_EQ(list.size(), 0);
    EXPECT_TRUE(list.read().empty());
    list.insertHead("b");
    list.insertBefore("b", "a");
    list.insertAfter("b", "d");
    list.insertBefore("d", "c");
    list.insertAfter("a", "x");
    list.push("b");
    list.insertTail("e");
    EXPECT_EQ(list.size(), 7);
    EXPECT_EQ(list.get("c"), "c");
    EXPECT_EQ(list.find("b"), 2);
    EXPECT_EQ(list.find("missing"), -1);
    EXPECT_EQ(list.read(), (vector<string>{"a", "x", "b", "c", "d", "b", "e"}));
    EXPECT_EQ(list.read(true), (vector<string>{"e", "b", "d", "c", "b", "x", "a"}));
    EXPECT_EQ(list.deleteAfter("a"), "x");
    EXPECT_EQ(list.deleteBefore("b"), "a");
    EXPECT_EQ(list.deleteBefore("d"), "c");
    EXPECT_EQ(list.eraseValue("b"), "b");
    EXPECT_EQ(list.deleteTail(), "e");
    EXPECT_EQ(list.remove("b"), "b");
    EXPECT_EQ(list.deleteHead(), "d");
    EXPECT_EQ(list.size(), 0);
    list.insertTail("new");
    EXPECT_EQ(list.deleteTail(), "new");
    list.insertHead("again");
    EXPECT_EQ(list.read(true), (vector<string>{"again"}));
}

TYPED_TEST(ListTest, RejectsMissingValuesAndNeighbours) {
    TypeParam list;
    EXPECT_THROW(list.deleteHead(), std::runtime_error);
    EXPECT_THROW(list.deleteTail(), std::runtime_error);
    list.push("only");
    EXPECT_THROW(list.get("missing"), std::runtime_error);
    EXPECT_THROW(list.insertBefore("missing", "x"), std::runtime_error);
    EXPECT_THROW(list.insertAfter("missing", "x"), std::runtime_error);
    EXPECT_THROW(list.eraseValue("missing"), std::runtime_error);
    EXPECT_THROW(list.deleteBefore("missing"), std::runtime_error);
    EXPECT_THROW(list.deleteAfter("missing"), std::runtime_error);
    EXPECT_THROW(list.deleteBefore("only"), std::runtime_error);
    EXPECT_THROW(list.deleteAfter("only"), std::runtime_error);
    EXPECT_EQ(list.read(), (vector<string>{"only"}));
}

TEST(QueueTest, PopsInFIFOOrderAndSupportsReuse) {
    Queue queue;
    EXPECT_TRUE(queue.read().empty());
    EXPECT_THROW(queue.get(), std::runtime_error);
    EXPECT_THROW(queue.pop(), std::runtime_error);
    queue.push("a");
    queue.push("b");
    EXPECT_EQ(queue.get(), "a");
    EXPECT_EQ(queue.read(), (vector<string>{"a", "b"}));
    EXPECT_EQ(queue.pop(), "a");
    queue.push("c");
    EXPECT_EQ(queue.pop(), "b");
    EXPECT_EQ(queue.pop(), "c");
    queue.push("new");
    EXPECT_EQ(queue.read(), (vector<string>{"new"}));
}

TEST(StackTest, PopsInLIFOOrderAndSupportsReuse) {
    Stack stack;
    EXPECT_TRUE(stack.read().empty());
    EXPECT_THROW(stack.get(), std::runtime_error);
    EXPECT_THROW(stack.pop(), std::runtime_error);
    stack.push("a");
    stack.push("b");
    EXPECT_EQ(stack.get(), "b");
    EXPECT_EQ(stack.read(), (vector<string>{"b", "a"}));
    EXPECT_EQ(stack.pop(), "b");
    EXPECT_EQ(stack.pop(), "a");
    stack.push("new");
    EXPECT_EQ(stack.read(), (vector<string>{"new"}));
}

TEST(AvlTreeTest, PerformsAllFourRotations) {
    for (const auto& keys : vector<vector<int>>{{3, 2, 1}, {1, 2, 3}, {3, 1, 2}, {1, 3, 2}}) {
        SCOPED_TRACE(testing::PrintToString(keys));
        AvlTree tree;
        for (int key : keys) tree.insert(key);
        tree.insert(2);
        EXPECT_EQ(tree.values(), (vector<int>{1, 2, 3}));
        EXPECT_EQ(tree.readPreOrder(), "ROOT: 2\n  L: 1\n  R: 3");
        EXPECT_EQ(tree.levelOrder(), (vector<int>{2, 1, 3}));
    }
}

TEST(AvlTreeTest, ErasesLeafSingleChildAndTwoChildren) {
    AvlTree tree;
    EXPECT_FALSE(tree.contains(8));
    EXPECT_FALSE(tree.erase(8));
    EXPECT_EQ(tree.readPreOrder(), "(empty)");
    EXPECT_TRUE(tree.values().empty());
    EXPECT_TRUE(tree.levelOrder().empty());
    for (int key : {4, 2, 6, 1, 3, 5, 8, 7}) tree.insert(key);
    EXPECT_TRUE(tree.erase(4));
    EXPECT_TRUE(tree.erase(8));
    EXPECT_TRUE(tree.erase(1));
    EXPECT_FALSE(tree.contains(4));
    EXPECT_TRUE(tree.contains(7));
    EXPECT_EQ(tree.values(), (vector<int>{2, 3, 5, 6, 7}));
}

TEST(AvlTreeTest, RebalancesAfterDeletion) {
    AvlTree tree;
    for (int key : {4, 2, 5, 1, 3}) tree.insert(key);
    EXPECT_TRUE(tree.erase(5));
    EXPECT_EQ(tree.readPreOrder(), "ROOT: 2\n  L: 1\n  R: 4\n    L: 3");
}

class HashTableTest : public testing::TestWithParam<bool> {};

TEST_P(HashTableTest, HandlesCollisionsUpdatesAndGrowth) {
    HashTable table(GetParam());
    string result;
    auto initialCapacity = table.capacity();
    EXPECT_FALSE(table.get("missing", result));
    EXPECT_FALSE(table.erase("missing"));
    // These keys initially map to the same two buckets.
    EXPECT_TRUE(table.put("key25", "one"));
    EXPECT_TRUE(table.put("key44", "two"));
    EXPECT_TRUE(table.put("key52", "three"));
    EXPECT_FALSE(table.put("key44", "updated"));
    EXPECT_EQ(table.size(), 3U);
    EXPECT_EQ(table.keys(), (vector<string>{"key25", "key44", "key52"}));
    ASSERT_TRUE(table.get("key25", result));
    EXPECT_EQ(result, "one");
    ASSERT_TRUE(table.get("key44", result));
    EXPECT_EQ(result, "updated");
    ASSERT_TRUE(table.get("key52", result));
    EXPECT_EQ(result, "three");
    EXPECT_TRUE(table.erase("key25"));
    EXPECT_FALSE(table.erase("key25"));
    for (int i = 0; i < 500; ++i) {
        auto value = std::to_string(i);
        ASSERT_TRUE(table.put("item" + value, value));
    }
    EXPECT_GT(table.capacity(), initialCapacity);
    EXPECT_EQ(table.size(), 502U);
    for (int i = 0; i < 500; ++i) {
        auto value = std::to_string(i);
        auto key = "item" + value;
        ASSERT_TRUE(table.get(key, result)) << key;
        EXPECT_EQ(result, value);
        EXPECT_TRUE(table.erase(key));
    }
    EXPECT_EQ(table.keys(), (vector<string>{"key44", "key52"}));
}

INSTANTIATE_TEST_SUITE_P(Methods, HashTableTest, testing::Bool());
