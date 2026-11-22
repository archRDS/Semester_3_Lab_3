#include <stdexcept>
#include "double_list.h"

using namespace std;

DoubleList::Node* DoubleList::findNode(const string& value) const {
    for (Node* node = head; node != nullptr; node = node->next)
        if (node->value == value) return node;
    return nullptr;
}

string DoubleList::removeNode(Node* node) {
    if (node == nullptr) throw runtime_error("no element to delete");
    string value = node->value;
    if (node->prev == nullptr) head = node->next;
    else node->prev->next = node->next;
    if (node->next == nullptr) tail = node->prev;
    else node->next->prev = node->prev;
    delete node;
    length--;
    return value;
}

DoubleList::~DoubleList() {
    while (head != nullptr) {
        Node* next = head->next;
        delete head;
        head = next;
    }
}
int DoubleList::size() const { return length; }

void DoubleList::insertHead(const string& value) {
    Node* node = new Node{value, nullptr, head};
    if (head == nullptr) tail = node;
    else head->prev = node;
    head = node;
    length++;
}

void DoubleList::insertTail(const string& value) {
    Node* node = new Node{value, tail, nullptr};
    if (tail == nullptr) head = node;
    else tail->next = node;
    tail = node;
    length++;
}

void DoubleList::push(const string& value) { insertTail(value); }

void DoubleList::insertBefore(const string& target, const string& value) {
    Node* after = findNode(target);
    if (after == nullptr) throw runtime_error("value not found");
    if (after == head) { insertHead(value); return; }
    Node* node = new Node{value, after->prev, after};
    after->prev->next = node;
    after->prev = node;
    length++;
}

void DoubleList::insertAfter(const string& target, const string& value) {
    Node* before = findNode(target);
    if (before == nullptr) throw runtime_error("value not found");
    if (before == tail) { insertTail(value); return; }
    Node* node = new Node{value, before, before->next};
    before->next->prev = node;
    before->next = node;
    length++;
}

string DoubleList::deleteHead() { return removeNode(head); }
string DoubleList::deleteTail() { return removeNode(tail); }
string DoubleList::eraseValue(const string& target) {
    Node* node = findNode(target);
    if (node == nullptr) throw runtime_error("value not found");
    return removeNode(node);
}
string DoubleList::deleteBefore(const string& target) {
    Node* node = findNode(target);
    if (node == nullptr) throw runtime_error("value not found");
    if (node->prev == nullptr) throw runtime_error("no element before value");
    return removeNode(node->prev);
}
string DoubleList::deleteAfter(const string& target) {
    Node* node = findNode(target);
    if (node == nullptr) throw runtime_error("value not found");
    if (node->next == nullptr) throw runtime_error("no element after value");
    return removeNode(node->next);
}
string DoubleList::remove(const string& value) { return eraseValue(value); }
string DoubleList::get(const string& value) const {
    Node* node = findNode(value);
    if (node == nullptr) throw runtime_error("value not found");
    return node->value;
}
int DoubleList::find(const string& value) const {
    int position = 0;
    for (Node* node = head; node != nullptr; node = node->next, position++)
        if (node->value == value) return position;
    return -1;
}
vector<string> DoubleList::read(bool reverse) const {
    vector<string> result;
    if (reverse) {
        for (Node* node = tail; node != nullptr; node = node->prev) result.push_back(node->value);
    } else {
        for (Node* node = head; node != nullptr; node = node->next) result.push_back(node->value);
    }
    return result;
}
