#pragma once

/* C++ Standard Library */
#include <cstdint>
#include <format>
#include <ostream>
#include <string_view>

namespace machine::property
{

    /** @brief Number resolution. */
    /**
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
     * @see machine::property::Resolution::Kind
     */
    class Resolution
    {
    public:
        explicit Resolution() noexcept = delete; //!< @brief Default constructor (deleted).

    /* ^\__________________________________________ */
    /* #region Static members, Inner types.         */

    public:

        /** @brief The %resolution of the number. */
        /**
         * @details
         * %Resolution encoding (3-bit).
         *
         * @code
         * bit2 bit1 bit0
         *   ^    ^    ^
         *   |    |    '-- coefficient ( 0=x1, 1=x5 )
         *   '----'------- signed shift N (2-bit, 2's complement)
         * @endcode
         *
         * @par Examples:
         * - `X50   (0b'011) = 10^+1 × 5 = 50`
         * - `X1    (0b'000) = 10^+0 × 1 = 1`
         * - `X0_01 (0b'100) = 10^-2 × 1 = 0.01`
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

        /** @brief The number of bits used to represent @ref Kind. */
        static std::uint8_t constexpr KIND_BITS = 3U;

        /** @brief A mask to extract the @ref Kind ​​from the `std::uint8_t`. */
        static std::uint8_t constexpr KIND_MASK = ( 1U << KIND_BITS ) - 1U;

        /** @brief Convert raw 3-bit value to @ref Kind. */
        /**
         * @details
         * Converts the given raw value (lower 3 bits) into a corresponding
         * @ref Kind value.
         *
         * The input value is masked with @ref KIND_MASK to ensure that
         * only the valid resolution bits are used.
         *
         * @par Input / Output
         *
         * | raw (uint8_t) | masked  | Resulting Kind |
         * | ------------- | ------- | -------------- |
         * | 0b'0000'0000  | 0b'000  | Kind::X1       |
         * | 0b'0000'0001  | 0b'001  | Kind::X5       |
         * | 0b'0000'0010  | 0b'010  | Kind::X10      |
         * | 0b'0000'0011  | 0b'011  | Kind::X50      |
         * | 0b'0000'0100  | 0b'100  | Kind::X0_01    |
         * | 0b'0000'0101  | 0b'101  | Kind::X0_05    |
         * | 0b'0000'0110  | 0b'110  | Kind::X0_1     |
         * | 0b'0000'0111  | 0b'111  | Kind::X0_5     |
         *
         * @note ja: 下位3ビットを @ref Kind に変換します。
         *           入力値は @ref KIND_MASK によりマスクされます。
         *
         * @param raw [in] raw 3-bit encoded resolution value
         * @return corresponding @ref Kind.
         */
        [[nodiscard]] static constexpr Kind fromRaw( std::uint8_t const &raw ) noexcept
        {
            std::uint8_t const v = raw & KIND_MASK;
            return static_cast<Kind>( v );
        }

        /** @brief Returns the enumerator name of the given value. */
        /**
         * @details
         * Inputs and outputs are as follows:
         *
         * | INPUT       | OUTPUT  |
         * | ------------| ------- |
         * | Kind::X1    | `x1`    |
         * | Kind::X5    | `x5`    |
         * | Kind::X10   | `x10`   |
         * | Kind::X50   | `x50`   |
         * | Kind::X0_01 | `x0.01` |
         * | Kind::X0_05 | `x0.05` |
         * | Kind::X0_1  | `x0.1`  |
         * | Kind::X0_5  | `x0.5`  |
         */
        [[nodiscard]] static std::string_view constexpr nameOf( Kind const &v ) noexcept;

        /** @brief Extracts the signed exponent N of ScaleBy10Pow(N) from the given ​​value. */
        /**
         * @details
         * Inputs and outputs are as follows:
         *
         * | INPUT       | bit2, bit1 of INPUT value    | OUTPUT |
         * | ----------- | ---------------------------: | -----: |
         * | Kind::X1    | 0b'00 (+0 as 2's complement) |     +0 |
         * | Kind::X5    | 0b'00 (+0 as 2's complement) |     +0 |
         * | Kind::X10   | 0b'01 (+1 as 2's complement) |     +1 |
         * | Kind::X50   | 0b'01 (+1 as 2's complement) |     +1 |
         * | Kind::X0_01 | 0b'10 (-2 as 2's complement) |     -2 |
         * | Kind::X0_05 | 0b'10 (-2 as 2's complement) |     -2 |
         * | Kind::X0_1  | 0b'11 (-1 as 2's complement) |     -1 |
         * | Kind::X0_5  | 0b'11 (-1 as 2's complement) |     -1 |
         */
        [[nodiscard]] static std::int8_t constexpr shiftOf( Kind const &v ) noexcept;

        /** @brief Extracts the coefficient from the given ​​value. */
        /**
         * @details
         * Inputs and outputs are as follows:
         *
         * | INPUT       | bit0 of INPUT value | OUTPUT |
         * | ----------- | ------------------: | -----: |
         * | Kind::X1    | 0b'0                |      1 |
         * | Kind::X5    | 0b'1                |      5 |
         * | Kind::X10   | 0b'0                |      1 |
         * | Kind::X50   | 0b'1                |      5 |
         * | Kind::X0_01 | 0b'0                |      1 |
         * | Kind::X0_05 | 0b'1                |      5 |
         * | Kind::X0_1  | 0b'0                |      1 |
         * | Kind::X0_5  | 0b'1                |      5 |
         */
        [[nodiscard]] static std::uint8_t constexpr coeffOf( Kind const &v ) noexcept;

        /** @brief Get the real-valued scale factor of the given resolution. */
        /**
         * @details
         * This function returns the multiplicative scale factor represented
         * by the given @ref Kind.
         *
         * The scale factor is defined as:
         * @code
         *   scale = coefficient × 10^( shift )
         * @endcode
         *
         * This value can be used to convert a raw integer value into a
         * real-world quantity:
         * @code
         *   real_value = raw_value * scaleFactorOf( kind );
         * @endcode
         *
         * @par Input / Output
         *
         * | INPUT       | OUTPUT  |
         * | ----------- | ------: |
         * | Kind::X1    | ` 1.0 ` |
         * | Kind::X5    | ` 5.0 ` |
         * | Kind::X10   | `10.0 ` |
         * | Kind::X50   | `50.0 ` |
         * | Kind::X0_01 | ` 0.01` |
         * | Kind::X0_05 | ` 0.05` |
         * | Kind::X0_1  | ` 0.1 ` |
         * | Kind::X0_5  | ` 0.5 ` |
         *
         * @note
         * This function introduces floating-point semantics intentionally.
         * Low-level code may avoid calling this function and instead work
         * with integer arithmetic using @ref shiftOf and @ref coeffOf.
         *
         * @param v the `Kind`
         * @return real-valued scale factor
         */
        [[nodiscard]] static double constexpr scaleFactorOf( Kind const &v ) noexcept;

    /* #endregion */// Static members, Inner types

    }; // class Resolution

    /** @brief Stream output operator for `Resolution::Kind`. */
    /**
     * @details
     * Outputs the string representation of the `Resolution::Kind` instance
     * to the provided output stream.
     *
     * @see std::formatter<machine::property::Resolution::Kind> for formatting details.
     *
     * @param os The output stream to write to.
     * @param v The `Resolution::Kind` instance to output.
     *
     * @return Reference to the output stream after writing.
     */
    std::ostream &operator<<( std::ostream &os, Resolution::Kind const &v ) noexcept;

} // namespace machine::property

namespace std {

    /** @brief Formatter specialization for `machine::property::Resolution::Kind`. */
    /**
     * @details
     * Formats a `Kind` instance. Examples are follows:
     *
     * - `Kind::X1   `: `x1(0)`
     * - `Kind::X0_01`: `x0.01(4)`
     * - `Kind::X0_5 `: `x0.5(7)`
     */
    template <>
    struct formatter<machine::property::Resolution::Kind>;

} // namespace std
