#include "persistence.h"

#include <cstdint>
#include <istream>
#include <ostream>
#include <stdexcept>

namespace {

constexpr std::uint32_t maxItems = 1000000U;
constexpr std::uint32_t maxStringBytes = 16U * 1024 * 1024;

void write32(std::ostream& output, std::uint32_t value) {
    for (unsigned shift = 0; shift < 32; shift += 8) {
        output.put(static_cast<char>((value >> shift) & 0xffU));
    }
}

std::uint32_t read32(std::istream& input) {
    std::uint32_t value = 0;
    for (unsigned shift = 0; shift < 32; shift += 8) {
        int byte = input.get();
        if (byte == std::char_traits<char>::eof()) throw std::runtime_error("truncated integer");
        value |= static_cast<std::uint32_t>(byte) << shift;
    }
    return value;
}

void writeLength(std::ostream& output, std::size_t value, std::uint32_t limit) {
    if (value > limit) throw std::runtime_error("length exceeds format limit");
    write32(output, static_cast<std::uint32_t>(value));
}

std::uint32_t readLength(std::istream& input, std::uint32_t limit) {
    auto value = read32(input);
    if (value > limit) throw std::runtime_error("invalid length");
    return value;
}

void writeStrings(std::ostream& output, const std::vector<std::string>& values) {
    writeLength(output, values.size(), maxItems);
    for (const auto& value : values) {
        writeLength(output, value.size(), maxStringBytes);
        output.write(value.data(), static_cast<std::streamsize>(value.size()));
    }
}

std::vector<std::string> readStrings(std::istream& input) {
    auto count = readLength(input, maxItems);
    std::vector<std::string> values;
    for (std::uint32_t i = 0; i < count; ++i) {
        auto length = readLength(input, maxStringBytes);
        std::string value(length, '\0');
        if (!input.read(value.data(), length)) throw std::runtime_error("truncated string");
        values.push_back(std::move(value));
    }
    return values;
}

}  // namespace

void Lab1Data::writeBinary(std::ostream& output) const {
    output.write("LR3C", 4);
    writeStrings(output, array.read());
    writeStrings(output, single.read());
    writeStrings(output, doubly.read());
    writeStrings(output, queue.read());
    writeStrings(output, stack.read());
    auto keys = tree.levelOrder();
    writeLength(output, keys.size(), maxItems);
    for (int key : keys) write32(output, static_cast<std::uint32_t>(key));
    if (!output) throw std::runtime_error("binary write failed");
}

std::unique_ptr<Lab1Data> Lab1Data::readBinary(std::istream& input) {
    std::string header(4, '\0');
    if (!input.read(header.data(), 4) || header != "LR3C") {
        throw std::runtime_error("invalid binary header");
    }
    auto data = std::make_unique<Lab1Data>();
    for (const auto& value : readStrings(input)) data->array.push(value);
    for (const auto& value : readStrings(input)) data->single.push(value);
    for (const auto& value : readStrings(input)) data->doubly.push(value);
    for (const auto& value : readStrings(input)) data->queue.push(value);
    const auto stackValues = readStrings(input);
    for (auto it = stackValues.rbegin(); it != stackValues.rend(); ++it) data->stack.push(*it);
    auto count = readLength(input, maxItems);
    for (std::uint32_t i = 0; i < count; ++i) {
        auto raw = read32(input);
        auto key = raw <= INT32_MAX ? static_cast<std::int64_t>(raw)
                                   : static_cast<std::int64_t>(raw) - (INT64_C(1) << 32);
        if (data->tree.contains(static_cast<int>(key))) throw std::runtime_error("duplicate tree key");
        data->tree.insert(static_cast<int>(key));
    }
    if (input.peek() != std::char_traits<char>::eof() || input.bad()) {
        throw std::runtime_error("unexpected data after tree");
    }
    return data;
}
