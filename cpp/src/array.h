#pragma once

#include <memory>
#include <string>
#include <vector>

class Array {
public:
    Array() = default;
    ~Array() = default;
    Array(const Array&) = delete;
    Array& operator=(const Array&) = delete;

    int size() const;
    void push(const std::string& value);
    void insert(int index, const std::string& value);
    std::string get(int index) const;
    void set(int index, const std::string& value);
    std::string erase(int index);
    std::vector<std::string> read() const;

private:
    std::unique_ptr<std::string[]> data;
    int length = 0;
    int capacity = 0;
};
