#pragma once

#include <cstdint>
#include <string_view>
#include <array>

// TODO : Make the Resolution class and Kind enum;
namespace machine {

    namespace detail {

        /** @brief The names of each @ref Resolution ​​value. */
        constexpr std::array<std::string_view, 8> RESOLUTION_NAMES =
        {
            "x1",       "x5",
            "x10",      "x50",
            "x0.01",    "x0.05",
            "x0.1",     "x0.5",
        };

    } // namespace detail

    /** @brief The number of bits used to represent @ref Resolution. */
    constexpr uint8_t RESOLUTION_BITS = 3U;

    /** @brief A mask to extract the @ref Resolution ​​from the `uint8_t`. */
    constexpr uint8_t RESOLUTION_MASK = ( 1U << RESOLUTION_BITS ) - 1U;

    /** @brief The %resolution of the number. */
    /**
     * @details
     * %Resolution encoding (3-bit).
     * @code
     * bit2 bit1 bit0
     *   ^    ^    ^
     *   |    |    +-- coefficient ( 0=x1, 1=x5 )
     *   |    |
     *   +----+------- signed shift N (2-bit, 2's complement)
     * @endcode
     *
     * @par Examples:
     * - `X50   (0b011) = 10^+1 × 5 = 50`
     * - `X1    (0b000) = 10^+0 × 1 = 1`
     * - `X0_01 (0b100) = 10^-2 × 1 = 0.01`
     */
    enum class Resolution : std::uint8_t
    {
        X1    = 0b000, //!< `10^+0 x 1 = x1`
        X5    = 0b001, //!< `10^+0 x 5 = x5`
        X10   = 0b010, //!< `10^+1 x 1 = x10`
        X50   = 0b011, //!< `10^+1 x 5 = x50`
        X0_01 = 0b100, //!< `10^-2 x 1 = x0.01`
        X0_05 = 0b101, //!< `10^-2 x 5 = x0.05`
        X0_1  = 0b110, //!< `10^-1 x 1 = x0.1`
        X0_5  = 0b111, //!< `10^-1 x 5 = x0.5`
    };

    /** @brief Get the signed exponent N of ScaleBy10Pow(N) from the given ​​value. */
    /**
     * @details
     * Inputs and outputs are as follows:
     * | INPUT             | OUTPUT |
     * | ----------------- | -----: |
     * | Resolution::X1    |     -2 |
     * | Resolution::X5    |     -2 |
     * | Resolution::X10   |     -1 |
     * | Resolution::X50   |     -1 |
     * | Resolution::X0_01 |      0 |
     * | Resolution::X0_05 |      0 |
     * | Resolution::X0_1  |     +1 |
     * | Resolution::X0_5  |     +1 |
     */
    constexpr int8_t shift_of( Resolution v ) noexcept
    {
        /* 2-bit signed (two's complement) */
        return static_cast<int8_t>(
            ( static_cast<uint8_t>( v ) >> 1 ) & 0b11 ) - 2;
    }

    /** @brief Get the coefficient of the given ​​value. */
    /**
     * @details
     * Inputs and outputs are as follows:
     * | INPUT             | OUTPUT |
     * | ----------------- | -----: |
     * | Resolution::X1    |      1 |
     * | Resolution::X5    |      5 |
     * | Resolution::X10   |      1 |
     * | Resolution::X50   |      5 |
     * | Resolution::X0_01 |      1 |
     * | Resolution::X0_05 |      5 |
     * | Resolution::X0_1  |      1 |
     * | Resolution::X0_5  |      5 |
     */
    constexpr uint8_t coeff_of( Resolution v ) noexcept
    {
        return static_cast<uint8_t>(
            ( static_cast<uint8_t>( v ) & 0b1 ) ? 5 : 1 );
    }


    /** @brief Get the name of the given ​​value. */
    /** @details
     * Inputs and outputs are as follows:
     * | INPUT             | OUTPUT  |
     * | ----------------- | ------- |
     * | Resolution::X1    | "x1"    |
     * | Resolution::X5    | "x5"    |
     * | Resolution::X10   | "x10"   |
     * | Resolution::X50   | "x50"   |
     * | Resolution::X0_01 | "x0.01" |
     * | Resolution::X0_05 | "x0.05" |
     * | Resolution::X0_1  | "x0.1"  |
     * | Resolution::X0_5  | "x0.5"  |
     */
    constexpr std::string_view name_of( Resolution v ) noexcept
    {
        return detail::RESOLUTION_NAMES[static_cast<uint8_t>( v )];
    }
} // namespace machine

// namespace std {

//     /** @brief Formatter specialization for `machine::PropertySpec`. */
//     /**
//      * @details
//      * Formats a `PropertySpec` instance. Examples are follows:
//      * - Numeric: `PropertySpec{valueKind=Numeric, permission=ReadOnly, resolution=ShiftL1x5, initVal=0, minVal=-3, maxVal=3}`
//      * - Boolean: `PropertySpec{valueKind=Boolean, permission=ReadWrite, resolution=ShiftR0x1, initVal=false, minVal=0, maxVal=1}`
//      * @note This specialization is necessary because user-defined
//      * types do not have a default formatter in the standard library.
//      */
//     template <>
//     struct formatter<machine::PropertySpec> {
//         /** @brief Parse format specifiers (none supported). */
//         constexpr auto parse(std::format_parse_context &ctx) {
//             return ctx.begin();
//         }

//         /** @brief Format `machine::Component` value. */
//         /**
//          * @details
//          * Formats a `Component` instance to `Component{code=Number, index=Number}`.
//          */
//         template <typename FormatContext>
//         auto format(const machine::Property &v, FormatContext &ctx) const {
//             return std::format_to(ctx.out(), "Property{{code={}, index={}, level={}}}",
//                 v.code(), static_cast<int>(v.index()), static_cast<int>(v.level()));
//         }
//     };


// } // namespace std
