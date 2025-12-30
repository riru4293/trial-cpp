#pragma once

/* C++ Standard Library */
#include <cstdint>
#include <ostream>
#include <string_view>

namespace machine::property
{

    /** @brief Property value access permission. */
    /**
     * @details
     * The permission for the property value access, expressed in 2 bits.
     * See `Permission::Kind` for the definition.
     *
     * @note ja: 2ビットで表現されるプロパティ値へのアクセス権限。
     *           定義については `Permission::Kind` を参照してください。
     *
     * @see machine::property::Permission::Kind
     */
    class Permission
    {
    public:
        explicit Permission() noexcept = delete; //!< @brief Default constructor (deleted).

    /* ^\__________________________________________ */
    /* #region Static members, Inner types.         */

    public:

        /** @brief Kind of the permission for the property value access. */
        /**
         * @details
         * %Permission encoding (2-bit).
         *
         * @verbatim
           bit1 bit0
             ^    ^
             |    '-- write permission (0=denied, 1=allowed)
             '------- read  permission (0=denied, 1=allowed)
           @endverbatim
         */
        enum class Kind : std::uint8_t
        {
            None      = 0b00, //!< No access permission
            WriteOnly = 0b01, //!< Write-only access permission
            ReadOnly  = 0b10, //!< Read-only access permission
            ReadWrite = 0b11, //!< Read-write access permission
        };

        /** @brief Convert raw 2-bit value to @ref Kind. */
        /**
         * @details
         * Converts the given raw value (lower 2 bits) into a corresponding
         * @ref Kind value.
         *
         * The input value is masked with the lower 2 bits so that
         * only the valid permission bits are used.
         *
         * @par Input / Output
         *
         * | raw (uint8_t)  | masked  | OUTPUT            |
         * | -------------- | ------- | ----------------- |
         * | `0b'0000'0000` | `0b'00` | `Kind::None`      |
         * | `0b'0000'0001` | `0b'01` | `Kind::WriteOnly` |
         * | `0b'0000'0010` | `0b'10` | `Kind::ReadOnly`  |
         * | `0b'0000'0011` | `0b'11` | `Kind::ReadWrite` |
         *
         * @note ja: 下位2ビットを @ref Kind に変換します。
         *
         * @param raw [in] raw 2-bit encoded permission value
         *
         * @return corresponding @ref Kind
         */
        [[nodiscard]]
        static Kind fromRaw( std::uint8_t const &raw ) noexcept;

        /** @brief Returns the enumerator name of the given value. */
        /**
         * @details
         * Inputs and outputs are as follows:
         *
         * | INPUT             | OUTPUT       |
         * | ----------------- | ------------ |
         * | `Kind::None`      | `none`       |
         * | `Kind::WriteOnly` | `write-only` |
         * | `Kind::ReadOnly`  | `read-only`  |
         * | `Kind::ReadWrite` | `read-write` |
         *
         * @param v [in] the `Kind`
         *
         * @return enumerator name
         */
        [[nodiscard]]
        static std::string_view nameOf( Kind const &v ) noexcept;

    /* #endregion */// Static members, Inner types.

    }; // class Permission

    /** @brief Stream output operator for `Permission::Kind`. */
    /**
     * @details
     * Outputs the string representation of the `Permission::Kind` value
     * to the provided output stream.
     *
     * @see std::formatter<machine::property::Permission::Kind> for formatting details.
     *
     * @param os [out] The output stream to write to.
     * @param v  [in]  The `Permission::Kind` value to output.
     *
     * @return Reference to the output stream after writing.
     */
    std::ostream &operator<<( std::ostream &os, Permission::Kind const &v ) noexcept;

} // namespace machine::property
