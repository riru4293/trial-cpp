#pragma once

#include <cstdint>
#include <ostream>
#include <format>
#include <optional>
#include <string_view>
#include <value255.hpp>
#include <permission.hpp>
#include <resolution.hpp>
#include <format.hpp>
#include <value.hpp>

/* Custom Library */
#include <namespace.hpp>

namespace machine::property
{

    /** @brief Represents a property specification. */
    /**
     * @details
     * @par Overview:
     * A property specification consists of the following items:
     * 1. Value kind
     * 2. Value access permission
     * 3. Value resolution
     * 4. Initial value
     * 5. Minimum value
     * 6. Maximum value
     *
     * @par Value kind:
     * The value types are as follows; if no value exists, the value size is set to 0 bytes.
     * | Hex | Kind    | Details (value)             | Minimum value        | Maximum value          |
     * | --- | ------- | --------------------------- | -------------------- | ---------------------- |
     * | 0x0 | Numeric | Signed 1-4 bytes int        | Signed 1-4 bytes int | Signed 1-4 bytes int   |
     * | 0x1 | Boolean | 1 byte; 0=false, non-0=true | Fixed 0              | Fixed 1                |
     * | 0x2 | BitSet  | Unsigned 1-4 bytes int      | N/A                  | Unsigned 1-4 bytes int |
     * | 0x3 | String  | 1-192 ascii characters      | N/A                  | N/A                    |
     *
     * @par Initial value:
     * Default value used when no specific property value is provided, but a valid value is required.
     * 0-4 byte value, 0 byte indicates no value. If this value is not present, it defaults to 0.
     *
     * @note ja: 特定の値を設定できない場合でも、有効な値を必ず設定するための初期値。
     *           この値が存在しない場合、初期値は0とします。
     *
     * @par Minimum value:
     * The smallest valid value for the property value.
     *
     * @note ja: プロパティ値の最小有効値。
     *
     * @par Maximum value:
     * The largest valid value for the property value.
     * However, in the case of `BitSet`, it is used as a bitmask.
     * Its value has all enabled bits set to 1 and undefined bits set to 0.
     *
     * @note ja: プロパティ値の最大有効値。
     *           ただし、`BitSet`の場合はビットマスクとして使用します。
     *           その値は、定義済ビットは全て1、未定義ビットは0になっています。
     *
     * @par Hierarchy:
     * - Machine
     *   - `Unit[]` (unique: kind, index)
     *     - `Component[]` (unique: code, index)
     *       - `Property[]`  (unique: code)
     *         - `std::uint8_t code`
     *         - `value::Value255 value`
     *         - `machine::property::Spec spec` <---[this]
     */
    class Spec
    {
    /* ^\__________________________________________ */
    /* #region Static members, Inner types.         */

    public:




        /** @brief Returns a string representation of this. */
        /**
         * @details
         * The string representation is formatted as:
         * ```
         * { kind: <kind>, permission: <permission>, resolution: <resolution>,
         *   initial_value: <initial_value>, min_value: <min_value>, max_value: <max_value> }
         * ```
         *
         * @return String representation of this.
         */
        [[nodiscard]]
        std::string str() const noexcept
        {
            std::ostringstream oss;

            oss << "{ format: "       << name_of( kind() )
                << ", permission: " << Permission::name_of( permission() )
                << ", resolution: " << Resolution::name_of( resolution() )
                << ", init: "       << initVal_.str()
                << ", min: "        << minVal_.str()
                << ", max: "        << maxVal_.str()
                << " }";

            return oss.str();
        }

    private:


        /* #region Factory methods */

        /** @brief Create a PropertySpec instance with given parameters. */
        /**
         * @param perm value access permission
         * @param reso value resolution
         * @param init initial property value
         * @param min minimum property value
         * @param max maximum property value
         * @return PropertySpec instance if parameters are valid; std::nullopt otherwise.
         */
        static std::optional<PropertySpec> create(
            Perm perm,
            Reso reso,
            value::Value255 const init,
            value::Value255 const min,
            value::Value255 const max
        ) noexcept
        {
            auto cloned_init = init.clone();
            auto cloned_min = min.clone();
            auto cloned_max = max.clone();
            Fragments frags = {
                static_cast<std::uint8_t>( perm ), 0,
                static_cast<std::uint8_t>( reso ), 0
            };

            if ( cloned_init.has_value() &&
                 cloned_min.has_value()  &&
                 cloned_max.has_value() )
            {
                return std::optional<PropertySpec>{
                    PropertySpec{
                        std::move( frags ),
                        std::move( cloned_init.value() ),
                        std::move( cloned_min.value() ),
                        std::move( cloned_max.value() )
                    }
                };
            }

            return std::nullopt;
        }

        /* #endregion */// Factory methods

    private:
        struct Fragments
        {
            std::uint8_t format : 2;
            std::uint8_t permission: 2;
            std::uint8_t resolution: 3;
            std::uint8_t reserved : 1;
        };

    /* #endregion */// Static members, Inner types

    /* ^\__________________________________________ */
    /* #region Constructors.                        */

    public:

        /** @brief Constructor with given parameters. */
        /**
         * @param perm value access permission
         * @param reso value resolution
         * @param init_val initial property value
         * @param min_val minimum property value
         * @param max_val maximum property value
         */
        explicit PropertySpec(
            Perm perm, Reso reso, value::Value255 &&init_val,
            value::Value255 &&min_val, value::Value255 &&max_val
        ) noexcept :
            PropertySpec (
                {
                    static_cast<std::uint8_t>( perm ),
                    static_cast<std::uint8_t>( kind_of( min_val, max_val ) ),
                    static_cast<std::uint8_t>( reso ),
                    0
                },
                std::move( init_val ),
                std::move( min_val ),
                std::move( max_val )
            )
        { /* Do nothing */ }

        ~PropertySpec() noexcept = default;                   //!< Destructor (default).
        PropertySpec(const PropertySpec &) noexcept = delete; //!< Copy constructor (deleted).
        PropertySpec(PropertySpec &&) noexcept = default;     //!< Move constructor (default).

    private:

        explicit PropertySpec(
            Fragments       &&frags,
            value::Value255 &&init_val,
            value::Value255 &&min_val,
            value::Value255 &&max_val
        ) noexcept :
            frags_ ( std::move( frags   ) ), initVal_( std::move( init_val ) ),
            minVal_( std::move( min_val ) ), maxVal_ ( std::move( max_val  ) )
        { /* Do nothing */ }

    /* #endregion */// Constructors

    /* ^\__________________________________________ */
    /* #region Operators.                           */

    public:

        PropertySpec &operator=( PropertySpec const & ) noexcept = delete;          //!< Copy operator (deleted).
        PropertySpec &operator=( PropertySpec && ) noexcept = default;              //!< Move operator (default).
        bool constexpr operator==( PropertySpec const & ) const noexcept = delete;  //!< Equality operator (deleted).
        auto constexpr operator<=>( PropertySpec const & ) const noexcept = delete; //!< Three-way comparison operator (deleted).

    /* #endregion */// Operators

    /* ^\__________________________________________ */
    /* #region Instance members.                    */

    public:

        /* #region Getter methods */

        [[nodiscard]]
        Format::Kind format() const noexcept
        {
            return Format::fromRaw( frags_.kind );
        }

        [[nodiscard]]
        Permission::Kind permission() const noexcept
        {
            return Permission::fromRaw( frags_.perm );
        }

        [[nodiscard]]
        Resolution::Kind resolution() const noexcept
        {
            return Resolution::fromRaw( frags_.reso );
        }

        [[nodiscard]]
        Value const &initVal() const noexcept { return initVal_; }

        [[nodiscard]]
        Value const &minVal() const noexcept { return minVal_; }

        [[nodiscard]]
        Value const &maxVal() const noexcept { return maxVal_; }

        /* #endregion */

    private:

        /* #region : member variables */

        Fragments frags_;   // 1 bytes
        Value initVal_;     // 6 bytes
        Value minVal_;      // 6 bytes
        Value maxVal_;      // 6 bytes
        // ---------------------------------
        //             Total: 19 bytes

        /* #endregion */

    }; // class PropertySpec

    /** @brief Stream output operator for `Spec`. */
    /**
     * @details
     * Outputs the string representation of the `Spec` instance
     * to the provided output stream.
     *
     * @see Spec::name_of() for the format of the output.
     *
     * @param os The output stream to write to.
     * @param v The `Spec::Kind` instance to output.
     *
     * @return Reference to the output stream after writing.
     */
    std::ostream &operator<<( std::ostream &os, Spec::Kind const &v ) noexcept;

    /* ^\__________________________________________ */
    /* Static assertions.                           */
    static_assert( sizeof(machine::property::Spec) == 19, "Unexpected Spec size" );
    static_assert( alignof(machine::property::Spec) == 1, "Unexpected Spec alignment" );

} // namespace machine

namespace std
{

    /** @brief Formatter specialization for `machine::property::Spec`. */
    /**
     * @details
     * Formats a `machine::property::Spec` instance. Examples are follows:
     *
     * - A `Spec` with format=`Numeric`, permission=`ReadWrite`, resolution=`X1`,
     *   initial_value=`10`, minimum_value=`0`, maximum_value=`1024`
     *   will be formatted as:
     *   `{ format: numeric(0), permission: read-write(3), resolution: x1(0),
     *     initial_value: [ 0x0A ], minimum_value: [ 0x00 ], maximum_value: [ 0x00 0x04 ] }`
     */
    template <>
    struct formatter<machine::property::Spec>
    {
        using Spec = machine::property::Spec;

        /** @brief Parse format specifiers (none supported). */
        /**
         * @param ctx [in,out] The format parse context.
         *
         * @return Iterator pointing to the next character to be parsed
         *         (no specifiers are consumed).
         */
        constexpr auto parse( std::format_parse_context &ctx )
        {
            return ctx.begin();
        }

        /** @brief Format `Spec` value. */
        /**
         * @param v   [in]     The `Spec` value to format.
         * @param ctx [in,out] The format context.
         *
         * @return Iterator to the end of the formatted output.
         */
        template <typename FormatContext>
        auto format( Spec const &v, FormatContext &ctx ) const
        {
            return std::format_to( ctx.out(),
                R"({{ )"
                    R"(format: {}, permission: {}, resolution: {})"
                    R"(, initial_value: {}, minimum_value: {}, maximum_value: {})"
                R"( }})",
                v.format(), v.permission(), v.resolution(),
                v.initVal(), v.minVal(), v.maxVal()
            );
        }
    };

} // namespace std
