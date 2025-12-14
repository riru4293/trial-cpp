#pragma once

#include <cstddef>
#include <cstdint>

namespace util {
    namespace detail {
        /** @brief Golden ratio constant for 64-bit hash combination. */
        /** 
         * @details
         * This constant is used in the hash combination algorithm
         * to help distribute hash values uniformly.
         */
        constexpr std::uint64_t HASH_GOLDEN_RATIO_64 = 0x9e3779b97f4a7c15ULL;

        /** @brief Combine two hash values into one. */
        /** 
         * @param seed Current combined hash value.
         * @param value New hash value to combine.
         * @return Combined hash value.
         * @details
         * This function combines two hash values using a mixing function
         * that incorporates the golden ratio constant to reduce collisions.
         */
        constexpr std::size_t combinePair(std::size_t seed, std::size_t value) noexcept {
            return seed ^ (value + HASH_GOLDEN_RATIO_64 + (seed << 6) + (seed >> 2));
        }

        /** @brief Recursively combine multiple hash values. */
        /** 
         * @tparam Ts Types of the values to be hashed.
         * @param seed Current combined hash value.
         * @param values Values to be hashed.
         * @return Combined hash value.
         * @details
         * This function uses fold expressions (C++17) to combine
         * multiple hash values into a single hash value.
         */
        template <typename... Ts>
        constexpr std::size_t combineAll(std::size_t seed, Ts... values) noexcept {
            ((seed = combinePair(seed, values)), ...);
            return seed;
        }
    } // namespace util::detail

    /** @brief Combine multiple hash values into a single hash value. */
    /**
     * @tparam Ts Types of the values to be hashed.
     * @param values Values to be hashed.
     * @return Combined hash value.
     * @details
     * This function takes multiple values, computes their individual hash values,
     * and combines them into a single hash value using a mixing function.
     * @note All types passed to this function must be convertible to `std::size_t`.
     */
    template <typename... Ts>
    constexpr std::size_t makeHash(Ts... values) noexcept {
        static_assert(std::conjunction<std::is_convertible<Ts, std::size_t>...>::value,
                      "makeHash: all arguments must be convertible to std::size_t");

        std::size_t seed = 0;
        return detail::combineAll(seed, values...);
    }
} // namespace util
