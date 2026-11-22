#include "hash_table.h"

#include <algorithm>
#include <istream>
#include <ostream>
#include <stdexcept>
#include <utility>

namespace {

std::string hexEncode(const std::string& value) {
    const char digits[] = "0123456789abcdef";
    std::string result;
    for (unsigned char byte : value) {
        result += digits[byte >> 4];
        result += digits[byte & 15];
    }
    return result;
}

unsigned hexDigit(char digit) {
    if (digit >= '0' && digit <= '9') return static_cast<unsigned>(digit - '0');
    if (digit >= 'a' && digit <= 'f') return static_cast<unsigned>(digit - 'a' + 10);
    if (digit >= 'A' && digit <= 'F') return static_cast<unsigned>(digit - 'A' + 10);
    throw std::runtime_error("invalid hexadecimal digit");
}

std::string hexDecode(const std::string& value) {
    if (value.size() % 2 != 0) throw std::runtime_error("invalid hexadecimal length");
    std::string result;
    for (std::size_t i = 0; i < value.size(); i += 2) {
        result += static_cast<char>((hexDigit(value[i]) << 4) | hexDigit(value[i + 1]));
    }
    return result;
}

void trimCarriageReturn(std::string& line) {
    if (!line.empty() && line.back() == '\r') line.pop_back();
}

}  // namespace

HashTable::HashTable(bool useCuckoo) : cuckoo(useCuckoo) {
    if (cuckoo) {
        first.resize(11);
        second.resize(11);
    } else {
        chains.resize(11);
    }
}

std::size_t HashTable::size() const {
    return count;
}

std::size_t HashTable::capacity() const {
    return cuckoo ? first.size() * 2 : chains.size();
}

std::string HashTable::method() const {
    return cuckoo ? "cuckoo" : "chain";
}

std::uint64_t HashTable::hash(const std::string& key, std::uint64_t salt) {
    std::uint64_t value = 1469598103934665603ULL ^ salt;
    for (unsigned char byte : key) {
        value ^= byte;
        value *= 1099511628211ULL;
    }
    value ^= value >> 33;
    value *= 0xff51afd7ed558ccdULL;
    value ^= value >> 33;
    return value;
}

std::size_t HashTable::index(const std::string& key, int table) const {
    auto slots = cuckoo ? first.size() : chains.size();
    return hash(key, seed + (table == 1 ? 0x9e3779b97f4a7c15ULL : 0)) % slots;
}

std::optional<HashTable::Entry>* HashTable::slot(const std::string& key, int table) {
    return &(table == 0 ? first : second)[index(key, table)];
}

const std::optional<HashTable::Entry>* HashTable::slot(const std::string& key, int table) const {
    return &(table == 0 ? first : second)[index(key, table)];
}

std::vector<HashTable::Entry> HashTable::entries() const {
    std::vector<Entry> result;
    result.reserve(count);
    if (cuckoo) {
        for (const auto& entry : first) {
            if (entry) result.push_back(*entry);
        }
        for (const auto& entry : second) {
            if (entry) result.push_back(*entry);
        }
    } else {
        for (const auto& chain : chains) result.insert(result.end(), chain.begin(), chain.end());
    }
    return result;
}

void HashTable::growChains() {
    std::vector<std::vector<Entry>> old(chains.size() * 2 + 1);
    old.swap(chains);
    count = 0;
    for (const auto& chain : old) {
        for (const auto& entry : chain) put(entry.key, entry.value);
    }
}

std::optional<HashTable::Entry> HashTable::insertRaw(Entry entry) {
    int table = 0;
    for (std::size_t i = 0; i < 4 * first.size(); ++i) {
        auto& place = *slot(entry.key, table);
        if (!place) {
            place = std::move(entry);
            ++count;
            return std::nullopt;
        }
        std::swap(entry, *place);
        table ^= 1;
    }
    return entry;
}

void HashTable::rebuild(std::vector<Entry> items, std::size_t newCapacity) {
    for (;;) {
        first.assign(newCapacity, std::nullopt);
        second.assign(newCapacity, std::nullopt);
        count = 0;
        ++seed;
        bool needsRetry = false;
        for (auto current = items.begin(); current != items.end(); ++current) {
            if (auto displaced = insertRaw(*current)) {
                std::vector<Entry> retry(current + 1, items.end());
                retry.push_back(std::move(*displaced));
                auto placed = entries();
                retry.insert(retry.end(), placed.begin(), placed.end());
                items = std::move(retry);
                needsRetry = true;
                break;
            }
        }
        if (!needsRetry) return;
        newCapacity = newCapacity * 2 + 1;
    }
}

bool HashTable::get(const std::string& key, std::string& value) const {
    if (cuckoo) {
        for (int table = 0; table < 2; ++table) {
            const auto& entry = *slot(key, table);
            if (entry && entry->key == key) {
                value = entry->value;
                return true;
            }
        }
    } else {
        for (const auto& entry : chains[index(key, 0)]) {
            if (entry.key == key) {
                value = entry.value;
                return true;
            }
        }
    }
    return false;
}

bool HashTable::put(const std::string& key, const std::string& value) {
    if (cuckoo) {
        for (int table = 0; table < 2; ++table) {
            auto& entry = *slot(key, table);
            if (entry && entry->key == key) {
                entry->value = value;
                return false;
            }
        }
        if ((count + 1) * 2 > capacity()) {
            auto items = entries();
            items.push_back({key, value});
            rebuild(std::move(items), first.size() * 2 + 1);
            return true;
        }
        if (auto displaced = insertRaw({key, value})) {
            auto items = entries();
            items.push_back(std::move(*displaced));
            rebuild(std::move(items), first.size() * 2 + 1);
        }
        return true;
    }
    for (auto& entry : chains[index(key, 0)]) {
        if (entry.key == key) {
            entry.value = value;
            return false;
        }
    }
    if ((count + 1) * 10 > chains.size() * 9) growChains();
    chains[index(key, 0)].push_back({key, value});
    ++count;
    return true;
}

bool HashTable::erase(const std::string& key) {
    if (cuckoo) {
        for (int table = 0; table < 2; ++table) {
            auto& entry = *slot(key, table);
            if (entry && entry->key == key) {
                entry.reset();
                --count;
                return true;
            }
        }
    } else {
        auto& chain = chains[index(key, 0)];
        for (auto entry = chain.begin(); entry != chain.end(); ++entry) {
            if (entry->key == key) {
                chain.erase(entry);
                --count;
                return true;
            }
        }
    }
    return false;
}

std::vector<std::string> HashTable::keys() const {
    std::vector<std::string> result;
    for (const auto& entry : entries()) result.push_back(entry.key);
    std::sort(result.begin(), result.end());
    return result;
}

void HashTable::writeText(std::ostream& output) const {
    auto items = entries();
    std::sort(items.begin(), items.end(), [](const Entry& a, const Entry& b) {
        return a.key < b.key;
    });
    output << "LR3T " << method() << '\n';
    for (const auto& entry : items) {
        output << hexEncode(entry.key) << '\t' << hexEncode(entry.value) << '\n';
    }
    if (!output) throw std::runtime_error("text write failed");
}

HashTable HashTable::readText(std::istream& input) {
    std::string line;
    std::getline(input, line);
    trimCarriageReturn(line);
    if (line != "LR3T chain" && line != "LR3T cuckoo") {
        throw std::runtime_error("invalid text header");
    }
    HashTable table(line == "LR3T cuckoo");
    while (std::getline(input, line)) {
        trimCarriageReturn(line);
        auto separator = line.find('\t');
        if (separator == std::string::npos) throw std::runtime_error("missing key/value separator");
        auto key = hexDecode(line.substr(0, separator));
        auto value = hexDecode(line.substr(separator + 1));
        if (!table.put(key, value)) throw std::runtime_error("duplicate hash key");
    }
    if (!input.eof() || input.bad()) throw std::runtime_error("text read failed");
    return table;
}
