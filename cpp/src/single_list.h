#pragma once

#include <string>
#include <vector>

class SingleList {
public:
    SingleList() = default;
    ~SingleList();
    SingleList(const SingleList&) = delete;
    SingleList& operator=(const SingleList&) = delete;

    int size() const;
    void insertHead(const std::string& value);
    void insertTail(const std::string& value);
    void push(const std::string& value);
    void insertBefore(const std::string& target, const std::string& value);
    void insertAfter(const std::string& target, const std::string& value);
    std::string deleteHead();
    std::string deleteTail();
    std::string eraseValue(const std::string& target);
    std::string deleteBefore(const std::string& target);
    std::string deleteAfter(const std::string& target);
    std::string remove(const std::string& value);
    std::string get(const std::string& value) const;
    int find(const std::string& value) const;
    std::vector<std::string> read(bool reverse = false) const;

private:
    struct Node {
        std::string value;
        Node* next = nullptr;
    };

    Node* head = nullptr;
    Node* tail = nullptr;
    int length = 0;

    Node* findNode(const std::string& value) const;
    std::string removeAfter(Node* before);
};
