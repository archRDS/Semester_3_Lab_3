#include "hash_table.h"
#include "persistence.h"

#include <gtest/gtest.h>

#include <sstream>
#include <stdexcept>
#include <string>
#include <vector>

using std::string;
using std::vector;

TEST(BinaryPersistenceTest, RestoresValuesOrderAndTreeShape) {
    Lab1Data data;
    data.array.push(string("a\0b", 3));
    data.array.push("");
    data.single.push("one");
    data.single.push("two");
    data.doubly.push("left");
    data.doubly.push("right");
    data.queue.push("first");
    data.queue.push("second");
    data.stack.push("bottom");
    data.stack.push("top");
    for (int key : {0, -2, 2, -3, -1, 1, 3}) data.tree.insert(key);

    std::stringstream stream;
    data.writeBinary(stream);
    auto decoded = Lab1Data::readBinary(stream);
    EXPECT_EQ(decoded->array.read(), (vector<string>{string("a\0b", 3), ""}));
    EXPECT_EQ(decoded->single.read(), (vector<string>{"one", "two"}));
    EXPECT_EQ(decoded->doubly.read(true), (vector<string>{"right", "left"}));
    EXPECT_EQ(decoded->queue.pop(), "first");
    EXPECT_EQ(decoded->queue.pop(), "second");
    EXPECT_EQ(decoded->stack.pop(), "top");
    EXPECT_EQ(decoded->stack.pop(), "bottom");
    EXPECT_EQ(decoded->tree.values(), (vector<int>{-3, -2, -1, 0, 1, 2, 3}));
    EXPECT_EQ(decoded->tree.levelOrder(), data.tree.levelOrder());
}

TEST(BinaryPersistenceTest, RejectsTruncationAndInvalidLengths) {
    Lab1Data data;
    data.array.push("abc");
    data.tree.insert(4);
    data.tree.insert(5);
    std::ostringstream output;
    data.writeBinary(output);
    const string raw = output.str();
    for (std::size_t n = 0; n < raw.size(); ++n) {
        SCOPED_TRACE(n);
        std::istringstream input(raw.substr(0, n));
        EXPECT_THROW(Lab1Data::readBinary(input), std::runtime_error);
    }
    string duplicate = raw;
    duplicate.replace(duplicate.size() - 4, 4, raw.substr(raw.size() - 8, 4));
    for (const string& bad : {string("WRNG"), string("LR3C\xff\xff\xff\xff", 8), raw + "x", duplicate}) {
        std::istringstream input(bad);
        EXPECT_THROW(Lab1Data::readBinary(input), std::runtime_error);
    }
    std::ostringstream failed;
    failed.setstate(std::ios::badbit);
    EXPECT_THROW(data.writeBinary(failed), std::runtime_error);
}

TEST(TextPersistenceTest, RestoresBothHashMethodsAndSpecialBytes) {
    for (bool cuckoo : {false, true}) {
        HashTable table(cuckoo);
        table.put("", "");
        table.put("a", "b");
        table.put("key\t", string("value\n\0", 7));
        std::stringstream stream;
        table.writeText(stream);
        auto restored = HashTable::readText(stream);
        EXPECT_EQ(restored.method(), table.method());
        EXPECT_EQ(restored.keys(), table.keys());
        string value;
        ASSERT_TRUE(restored.get("key\t", value));
        EXPECT_EQ(value, string("value\n\0", 7));
        std::ostringstream replay;
        restored.writeText(replay);
        EXPECT_EQ(replay.str(), stream.str());
    }
    std::istringstream crlf("LR3T chain\r\n6A\t62\r\n");
    auto table = HashTable::readText(crlf);
    EXPECT_EQ(table.keys(), (vector<string>{"j"}));
}

TEST(TextPersistenceTest, RejectsInvalidDataAndWriteErrors) {
    for (const char* bad : {"wrong", "LR3T chain\n61", "LR3T chain\n6\t62",
                            "LR3T chain\nxx\t62", "LR3T chain\n61\t62\n61\t63"}) {
        std::istringstream input(bad);
        EXPECT_THROW(HashTable::readText(input), std::runtime_error);
    }
    HashTable text;
    std::ostringstream failed;
    failed.setstate(std::ios::badbit);
    EXPECT_THROW(text.writeText(failed), std::runtime_error);
}
