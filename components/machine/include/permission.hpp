#pragma once

/* C++ Standard Library */
#include <cstdint>
#include <format>
#include <string_view>

namespace machine
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
     * @see machine::Permission::Kind
     */
    class Permission
    {
    /* ^\__________________________________________ */
    /* #region Static members, Inner types.         */

    public:
        /** @brief The permission for the property value access. */
        /**
         * @details
         * %Permission encoding (2-bit).
         * @code
         * bit1 bit0
         *   ^    ^
         *   |    '-- write permission (0=denied, 1=allowed)
         *   '------- read  permission (0=denied, 1=allowed)
         * @endcode
         */
        enum class Kind : std::uint8_t
        {
            None      = 0b00, //!< No access permission
            WriteOnly = 0b01, //!< Write-only access permission
            ReadOnly  = 0b10, //!< Read-only access permission
            ReadWrite = 0b11, //!< Read-write access permission
        };

        /** @brief The number of bits used to represent @ref Permission. */
        static std::uint8_t constexpr PERMISSION_BITS = 2U;

        /** @brief A mask to extract the @ref Permission ​​from the `std::uint8_t`. */
        static std::uint8_t constexpr PERMISSION_MASK = ( 1U << PERMISSION_BITS ) - 1U;

        /** @brief Get the name of the given ​​value. */
        /** @details
          * Inputs and outputs are as follows:
          * | INPUT           | OUTPUT      |
          * | --------------- | ----------- |
          * | Kind::None      | "none"      |
          * | Kind::WriteOnly | "write-only"|
          * | Kind::ReadOnly  | "read-only" |
          * | Kind::ReadWrite | "read-write"|
          */
        [[nodiscard]] static std::string_view constexpr name_of( Kind v ) noexcept;

    /* #endregion */// Static members, Inner types.

    }; // class Permission

    /** @brief Alias of the @ref Permission::Kind */
    using Perm = Permission::Kind;

} // namespace machine

namespace std
{
    /** @brief Formatter specialization for `machine::Permission::Kind`. */
    /**
     * @details
     * Formats a `Kind` instance. Examples are follows:
     * - `Kind::None     `: `none(0)`
     * - `Kind::WriteOnly`: `write-only(1)`
     * - `Kind::ReadOnly `: `read-only(2)`
     * - `Kind::ReadWrite`: `read-write(3)`
     */
    template <>
    struct formatter<machine::Permission::Kind>
    {
        /** @brief Parse format specifiers (none supported). */
        constexpr auto parse( std::format_parse_context &ctx )
        {
            return ctx.begin();
        }

        /** @brief Format `machine::Permission::Kind` value. */
        template <typename FormatContext>
        auto format( machine::Permission::Kind const &v, FormatContext &ctx ) const
        {
            return std::format_to( ctx.out(), "{}({})",
                machine::Permission::name_of( v ), static_cast<int>( v ) );
        }
    };

} // namespace std
