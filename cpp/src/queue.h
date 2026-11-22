#pragma once

#include <string>
#include <vector>

class Queue {
public:
    Queue() = default;
    ~Queue();
    Queue(const Queue&) = delete;
    Queue& operator=(const Queue&) = delete;

    void push(const std::string& value);
    std::string get() const;
    std::string pop();
    std::vector<std::string> read() const;

private:
    struct Node {
        std::string value;
        Node* next = nullptr;
    };

    Node* head = nullptr;
    Node* tail = nullptr;
};
