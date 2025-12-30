#pragma once

/* C++ Standard Library */
#include <cstdint>
#include <ostream>
#include <string_view>
#include <string>

/* Custom Library */
#include <value.hpp>

namespace machine::property
{

    /** @brief Property value format. */
    /**
     * @details
     * Property value format, expressed in 2 bits.
     * See `Format::Kind` for the definition.
     *
     * @note ja: 2ビットで表現されるプロパティ値のフォーマット。
     *           定義については `Format::Kind` を参照してください。
     *
     * @see machine::property::Format::Kind
     */
    class Format
    {
    public:
        explicit Format() noexcept = delete; //!< @brief Default constructor (deleted).

    /* ^\__________________________________________ */
    /* #region Static members, Inner types.         */

    public:

        /** @brief Format kind of the property value. */
        enum class Kind : std::uint8_t {
            Numeric = 0, //!< Signed 1-4 bytes integer
            Boolean,     //!< 1 byte; 0=false, non-0=true
            BitSet,      //!< Unsigned 1-4 bytes integer representing bit set
            String,      //!< 1-192 bytes ASCII string
        };

        /** @brief Convert raw 2-bit value to @ref PropertyFormat::Kind. */
        /**
        * @details
        * Converts the given raw value (lower 2 bits) into a corresponding
        * @ref Kind value.
        *
        * The input value is masked with the lower 2 bits so that
        * only the valid resolution bits are used.
        *
        * @par Input / Output
        *
        * | raw (uint8_t)  | masked  | OUTPUT          |
        * | -------------- | ------- | --------------- |
        * | `0b'0000'0000` | `0b'00` | `Kind::Numeric` |
        * | `0b'0000'0001` | `0b'01` | `Kind::Boolean` |
        * | `0b'0000'0010` | `0b'10` | `Kind::BitSet`  |
        * | `0b'0000'0011` | `0b'11` | `Kind::String`  |
        *
        * @note ja: 下位2ビットを @ref Kind に変換します。
        *
        * @param raw [in] raw 2-bit encoded property value format value
        *
        * @return corresponding @ref Kind
        */
        [[nodiscard]]
        static Kind fromRaw( std::uint8_t raw ) noexcept;

        /** @brief Resolve a @ref PropertyFormat::Kind from property value range. */
        /**
            * @details
            * Determines the appropriate @ref Kind based on the provided
            * minimum and maximum property values.
            *
            * The resolution logic is as follows:
            *
            * - If both `min` and `max` have size 0, the kind is `String`.
            * - If `min` has size 0 and `max` has non-zero size, the kind is `BitSet`.
            * - If `min` and `max` both have size 1, and their byte values correspond
            *   to boolean values (0x00 for false and 0x01 for true), the kind is `Boolean`.
            * - In all other cases, the kind is `Numeric`.
            *
            * @param min [in] the minimum property value
            * @param max [in] the maximum property value
            *
            * @return the determined @ref Kind
         */
        [[nodiscard]]
        static Kind fromValueRange( Value const &min, Value const &max ) noexcept;

        /** @brief Returns the enumerator name of the given value. */
        /**
         * @details
         * Inputs and outputs are as follows:
         *
         * | INPUT           | OUTPUT    |
         * | --------------- | --------- |
         * | `Kind::Numeric` | `numeric` |
         * | `Kind::Boolean` | `boolean` |
         * | `Kind::BitSet`  | `bitset`  |
         * | `Kind::String`  | `string`  |
         *
         * @param v [in] the `Kind`
         *
         * @return enumerator name
         */
        [[nodiscard]]
        static std::string_view nameOf( Kind const &v ) noexcept;

        /** @brief Returns a string representation of the `Kind`. */
        /**
         * @details
         * Inputs and outputs are as follows:
         *
         * | INPUT           | OUTPUT       |
         * | --------------- | ------------ |
         * | `Kind::Numeric` | `numeric(0)` |
         * | `Kind::Boolean` | `boolean(1)` |
         * | `Kind::BitSet`  | `bitset(2)`  |
         * | `Kind::String`  | `string(3)`  |
         *
         * @param v [in] the `Kind`
         *
         * @return String representation of the `Kind`.
         */
        [[nodiscard]]
        static std::string strOf( Kind const &v ) noexcept;

    /* #endregion */// Static members, Inner types

    }; // class Format

    /** @brief Stream output operator for `Format::Kind`. */
    /**
     * @details
     * Outputs the string representation of the `Format::Kind` value
     * to the provided output stream.
     *
     * @see std::formatter<machine::property::Format::Kind> for formatting details.
     *
     * @param os [out] The output stream to write to.
     * @param v  [in]  The `Format::Kind` value to output.
     *
     * @return Reference to the output stream after writing.
     */
    std::ostream &operator<<( std::ostream &os, Format::Kind const &v ) noexcept;

} // namespace machine::property
