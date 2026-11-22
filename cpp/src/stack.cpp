#include <stdexcept>
#include "stack.h"

using namespace std;

Stack::~Stack() {
    while (top != nullptr) {
        Node* next = top->next;
        delete top;
        top = next;
    }
}

// Добавляет элемент на вершину стека.
void Stack::push(const string& value) {
    auto* node = new Node{value, top};
    top = node;
}

string Stack::get() const {
    if (top == nullptr) throw runtime_error("stack is empty");
    return top->value;
}

// Снимает элемент с вершины и возвращает его значение.
string Stack::pop() {
    string value = get();
    Node* old = top;
    top = old->next;
    delete old;
    return value;
}

vector<string> Stack::read() const {
    vector<string> result;
    for (Node* node = top; node != nullptr; node = node->next) {
        result.push_back(node->value);
    }
    return result;
}
