#pragma once

#include <cstdint>
#include <array>

namespace util {
    static constexpr int32_t toInt32LE(const std::array<std::byte, 3>& arr) {
        uint32_t value = toUInt32LE(arr);

        // Expand 24bit sign extension
        if (value & 0x00800000) { // Negative 24th bit is 1, indicating a negative number
            value |= 0xFF000000;  // Fill the upper 8 bits with 1s
        }
        return static_cast<int32_t>(value);
    }
    static constexpr uint32_t toUInt32LE(const std::array<std::byte, 3>& arr) {
        return
            (static_cast<uint32_t>(arr[0])      ) |
            (static_cast<uint32_t>(arr[1]) <<  8) |
            (static_cast<uint32_t>(arr[2]) << 16);
    }
} // namespace util
