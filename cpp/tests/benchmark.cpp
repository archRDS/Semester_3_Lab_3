#include "array.h"
#include "avl_tree.h"
#include "double_list.h"
#include "hash_table.h"
#include "queue.h"
#include "single_list.h"
#include "stack.h"

#include <chrono>
#include <cstddef>
#include <iomanip>
#include <iostream>

namespace {

template <class Operation>
void measure(const char* label, Operation operation) {
    constexpr std::size_t iterations = 100000;
    std::size_t checksum = 0;
    auto start = std::chrono::steady_clock::now();
    for (std::size_t i = 0; i < iterations; ++i) checksum += operation();
    auto elapsed = std::chrono::duration<double, std::nano>(std::chrono::steady_clock::now() - start);
    std::cout << std::left << std::setw(14) << label << std::right << std::fixed << std::setprecision(1)
              << std::setw(9) << elapsed.count() / iterations << " ns/op  " << checksum << '\n';
}

}  // namespace

int main() {
    Array array;
    SingleList single;
    DoubleList doubly;
    Queue queue;
    Stack stack;
    AvlTree tree;
    measure("array", [&] { array.push("value"); return array.erase(0).size(); });
    measure("single_list", [&] { single.insertHead("value"); return single.deleteHead().size(); });
    measure("double_list", [&] { doubly.insertHead("value"); return doubly.deleteHead().size(); });
    measure("queue", [&] { queue.push("value"); return queue.pop().size(); });
    measure("stack", [&] { stack.push("value"); return stack.pop().size(); });
    measure("AVL", [&] { tree.insert(42); return tree.erase(42); });

    for (bool cuckoo : {false, true}) {
        HashTable table(cuckoo);
        const auto method = table.method();
        measure(method.c_str(), [&] {
            std::string value;
            table.put("key", "value");
            table.get("key", value);
            table.erase("key");
            return value.size();
        });
    }
}
