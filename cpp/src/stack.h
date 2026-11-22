#pragma once

#include <string>
#include <vector>

class Stack {
public:
    Stack() = default;
    ~Stack();
    Stack(const Stack&) = delete;
    Stack& operator=(const Stack&) = delete;

    void push(const std::string& value);
    std::string get() const;
    std::string pop();
    std::vector<std::string> read() const;

private:
    struct Node {
        std::string value;
        Node* next = nullptr;
    };

    Node* top = nullptr;
};
