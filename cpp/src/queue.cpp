#include <stdexcept>
#include "queue.h"

using namespace std;

Queue::~Queue() {
    while (head != nullptr) {
        Node* next = head->next;
        delete head;
        head = next;
    }
}

// Добавляет новый элемент в конец очереди.
void Queue::push(const string& value) {
    Node* node = new Node{value};
    if (tail == nullptr) head = node;
    else tail->next = node;
    tail = node;
}

string Queue::get() const {
    if (head == nullptr) throw runtime_error("queue is empty");
    return head->value;
}

// Удаляет первый элемент очереди и возвращает его значение.
string Queue::pop() {
    string value = get();
    Node* old = head;
    head = old->next;
    if (head == nullptr) tail = nullptr;
    delete old;
    return value;
}

vector<string> Queue::read() const {
    vector<string> result;
    for (Node* node = head; node != nullptr; node = node->next) {
        result.push_back(node->value);
    }
    return result;
}
