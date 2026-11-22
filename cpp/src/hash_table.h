#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <optional>
#include <string>
#include <vector>

class HashTable {
public:
    explicit HashTable(bool useCuckoo = false);

    std::size_t size() const;
    std::size_t capacity() const;
    std::string method() const;
    bool get(const std::string& key, std::string& value) const;
    bool put(const std::string& key, const std::string& value);
    bool erase(const std::string& key);
    std::vector<std::string> keys() const;
    void writeText(std::ostream& output) const;
    static HashTable readText(std::istream& input);

private:
    struct Entry {
        std::string key;
        std::string value;
    };

    bool cuckoo;
    std::vector<std::vector<Entry>> chains;
    std::vector<std::optional<Entry>> first;
    std::vector<std::optional<Entry>> second;
    std::size_t count = 0;
    std::uint64_t seed = 0;

    static std::uint64_t hash(const std::string& key, std::uint64_t salt);
    std::size_t index(const std::string& key, int table) const;
    std::optional<Entry>* slot(const std::string& key, int table);
    const std::optional<Entry>* slot(const std::string& key, int table) const;
    void growChains();
    std::optional<Entry> insertRaw(Entry entry);
    void rebuild(std::vector<Entry> items, std::size_t newCapacity);
    std::vector<Entry> entries() const;
};
