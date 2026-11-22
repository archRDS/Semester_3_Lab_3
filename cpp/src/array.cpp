#include <stdexcept>
#include <utility>
#include "array.h"

using namespace std;

int Array::size() const {
    return length;
}

void Array::push(const string& value) {
    insert(length, value);
}

// При нехватке места увеличивает массив и вставляет значение по индексу.
void Array::insert(int index, const string& value) {
    if (index < 0 || index > length) throw runtime_error("index out of range");
    string newValue = value;
    if (length == capacity) {
        int newCapacity = capacity == 0 ? 4 : capacity * 2;
        auto newData = make_unique<string[]>(newCapacity);
        for (int i = 0; i < length; i++) newData[i] = data[i];
        data = std::move(newData);
        capacity = newCapacity;
    }
    for (int i = length; i > index; i--) data[i] = std::move(data[i - 1]);
    data[index] = std::move(newValue);
    length++;
}

string Array::get(int index) const {
    if (index < 0 || index >= length) throw runtime_error("index out of range");
    return data[index];
}

void Array::set(int index, const string& value) {
    if (index < 0 || index >= length) throw runtime_error("index out of range");
    data[index] = value;
}

// Удаляет элемент и сдвигает оставшиеся значения влево.
string Array::erase(int index) {
    string value = get(index);
    for (int i = index; i + 1 < length; i++) data[i] = std::move(data[i + 1]);
    data[--length].clear();
    return value;
}

vector<string> Array::read() const {
    vector<string> result;
    for (int i = 0; i < length; i++) result.push_back(data[i]);
    return result;
}
