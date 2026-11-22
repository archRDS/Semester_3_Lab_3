#include <algorithm>
#include <stdexcept>
#include "single_list.h"

using namespace std;

SingleList::Node* SingleList::findNode(const string& value) const {
    for (Node* node = head; node != nullptr; node = node->next)
        if (node->value == value) return node;
    return nullptr;
}

string SingleList::removeAfter(Node* before) {
    Node* node = before == nullptr ? head : before->next;
    if (node == nullptr) throw runtime_error("no element to delete");
    string value = node->value;
    if (before == nullptr) head = node->next;
    else before->next = node->next;
    if (node == tail) tail = before;
    delete node;
    length--;
    return value;
}

SingleList::~SingleList() {
    while (head != nullptr) {
        Node* next = head->next;
        delete head;
        head = next;
    }
}
int SingleList::size() const { return length; }

void SingleList::insertHead(const string& value) {
    auto* node = new Node{value, head};
    head = node;
    if (tail == nullptr) tail = head;
    length++;
}

void SingleList::insertTail(const string& value) {
    Node* node = new Node{value};
    if (tail == nullptr) head = node;
    else tail->next = node;
    tail = node;
    length++;
}

void SingleList::push(const string& value) { insertTail(value); }

void SingleList::insertBefore(const string& target, const string& value) {
    Node* before = nullptr;
    for (Node* node = head; node != nullptr; node = node->next) {
        if (node->value == target) {
            if (before == nullptr) insertHead(value);
            else { before->next = new Node{value, node}; length++; }
            return;
        }
        before = node;
    }
    throw runtime_error("value not found");
}

void SingleList::insertAfter(const string& target, const string& value) {
    Node* node = findNode(target);
    if (node == nullptr) throw runtime_error("value not found");
    node->next = new Node{value, node->next};
    if (tail == node) tail = node->next;
    length++;
}

string SingleList::deleteHead() { return removeAfter(nullptr); }
string SingleList::deleteTail() {
    if (head == nullptr) throw runtime_error("no element to delete");
    Node* before = nullptr;
    for (Node* node = head; node != tail; node = node->next) before = node;
    return removeAfter(before);
}

string SingleList::eraseValue(const string& target) {
    Node* before = nullptr;
    for (Node* node = head; node != nullptr; node = node->next) {
        if (node->value == target) return removeAfter(before);
        before = node;
    }
    throw runtime_error("value not found");
}

string SingleList::deleteBefore(const string& target) {
    Node* beforeBefore = nullptr;
    Node* before = nullptr;
    for (Node* node = head; node != nullptr; node = node->next) {
        if (node->value == target) {
            if (before == nullptr) throw runtime_error("no element before value");
            return removeAfter(beforeBefore);
        }
        beforeBefore = before;
        before = node;
    }
    throw runtime_error("value not found");
}

string SingleList::deleteAfter(const string& target) {
    Node* node = findNode(target);
    if (node == nullptr) throw runtime_error("value not found");
    return removeAfter(node);
}

string SingleList::remove(const string& value) { return eraseValue(value); }
string SingleList::get(const string& value) const {
    Node* node = findNode(value);
    if (node == nullptr) throw runtime_error("value not found");
    return node->value;
}
int SingleList::find(const string& value) const {
    int position = 0;
    for (Node* node = head; node != nullptr; node = node->next, position++)
        if (node->value == value) return position;
    return -1;
}
vector<string> SingleList::read(bool reverse) const {
    vector<string> result;
    for (Node* node = head; node != nullptr; node = node->next) result.push_back(node->value);
    if (reverse) std::reverse(result.begin(), result.end());
    return result;
}
