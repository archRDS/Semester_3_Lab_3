#pragma once

#include "array.h"
#include "avl_tree.h"
#include "double_list.h"
#include "queue.h"
#include "single_list.h"
#include "stack.h"

#include <iosfwd>
#include <memory>

struct Lab1Data {
    Array array;
    SingleList single;
    DoubleList doubly;
    Queue queue;
    Stack stack;
    AvlTree tree;

    void writeBinary(std::ostream& output) const;
    static std::unique_ptr<Lab1Data> readBinary(std::istream& input);
};
