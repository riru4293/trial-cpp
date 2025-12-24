#pragma once

#include <cstdint>
#include <string_view>
#include <format>

// TODO : Make the Resolution class and Kind enum;
namespace machine
{

    /**
     * @brief Number resolution.
     *
     * @details
     * The resolution of the number, expressed in 3 bits.
     * See `Resolution::Kind` for the definition.
     *
     * For example, a value resolution of 0x7 indicates a resolution of 5 * 10^-1 = 0.5.
     * As a concrete example, to represent a temperature of 25.5°C,
     * the value would be 51 and the resolution would be 0.5.
     *
     * @note ja: 数値の解像度。3ビットで表現されます。
     *           定義については `Resolution::Kind` を参照してください。
     *           例えば、解像度が0x7の場合、5 * 10^-1 = 0.5の解像度を示します。
     *           具体的な例として、25.5°Cの温度を表すには、値が51で解像度が0.5となります。
     *
     * @see machine::Resolution::Kind
     */
    class Resolution
    {
    /* ^\__________________________________________ */
    /* #region Static members, Inner types.         */

    public:

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
        enum class Kind : std::uint8_t
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

        /** @brief The number of bits used to represent @ref Resolution. */
        static uint8_t constexpr RESOLUTION_BITS = 3U;

        /** @brief A mask to extract the @ref Resolution ​​from the `uint8_t`. */
        static uint8_t constexpr RESOLUTION_MASK = ( 1U << RESOLUTION_BITS ) - 1U;

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
        static int8_t constexpr shift_of( Kind v ) noexcept;

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
        static uint8_t constexpr coeff_of( Kind v ) noexcept;

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
        static std::string_view constexpr name_of( Kind v ) noexcept;

        /**
        * @brief Get the real-valued scale factor of the given resolution.
        *
        * @details
        * This function returns the multiplicative scale factor represented
        * by the given @ref Resolution::Kind.
        *
        * The scale factor is defined as:
        * @code
        *   scale = coefficient × 10^(-shift)
        * @endcode
        *
        * This value can be used to convert a raw integer value into a
        * real-world quantity:
        * @code
        *   real_value = raw_value * Resolution::scale_factor(kind);
        * @endcode
        *
        * Examples:
        * - `Kind::X1`    → `1.0`
        * - `Kind::X5`    → `5.0`
        * - `Kind::X10`   → `10.0`
        * - `Kind::X50`   → `50.0`
        * - `Kind::X0_1`  → `0.1`
        * - `Kind::X0_5`  → `0.5`
        *
        * @note
        * This function introduces floating-point semantics intentionally.
        * Low-level code may avoid calling this function and instead work
        * with integer arithmetic using @ref shift_of and @ref coeff_of.
        *
        * @param v The resolution kind.
        * @return The real-valued scale factor.
        */
        static double scale_factor( Kind v ) noexcept;

    /* #endregion */// Static members, Inner types

    }; // class Resolution

} // namespace machine

namespace std {

    /** @brief Formatter specialization for `machine::Resolution::Kind`. */
    /**
     * @details
     * Formats a `Kind` instance. Examples are follows:
     * - `Kind::X1   `: `x1(0)`
     * - `Kind::X0_01`: `x0.01(4)`
     * - `Kind::X0_5 `: `x0.5(7)`
     */
    template <>
    struct formatter<machine::Resolution::Kind>
    {
        /** @brief Parse format specifiers (none supported). */
        constexpr auto parse( std::format_parse_context &ctx )
        {
            return ctx.begin();
        }

        /** @brief Format `machine::Resolution::Kind` value. */
        template <typename FormatContext>
        auto format( machine::Resolution::Kind const &v, FormatContext &ctx ) const
        {
            return std::format_to( ctx.out(), "{}{}{}{}",
                machine::Resolution::name_of(v), "(", static_cast<int>( v ), ")" );
        }
    };

} // namespace std
