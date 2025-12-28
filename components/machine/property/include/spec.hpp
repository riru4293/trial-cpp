#pragma once

#include <cstdint>
#include <ostream>
#include <format>
#include <optional>
#include <string_view>
#include <value255.hpp>
#include <permission.hpp>
#include <resolution.hpp>

/* Custom Library */
#include <namespace.hpp>

namespace machine {

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
     *         - `machine::PropertySpec spec` <---[this]
     */
    class PropertySpec
    {
    /* ^\__________________________________________ */
    /* #region Static members, Inner types.         */

    public:

        /** @brief The kind of the property value. */
        enum class Kind : std::uint8_t {
            Numeric, //!< Signed 1-3 byte integer
            Boolean, //!< 1 byte; 0=false, non-0=true
            BitSet,  //!< 1-3 byte
            String,  //!< 1-192 bytes
        };

        /** @brief The number of bits used to represent @ref Kind. */
        static std::uint8_t constexpr KIND_BITS = 2U;

        /** @brief A mask to extract the @ref PropertySpec::Kind ​​from the `std::uint8_t`. */
        static std::uint8_t constexpr KIND_MASK = ( 1U << KIND_BITS ) - 1U;

        /** @brief Get the name of the given ​​value. */
        /** @details
         * Inputs and outputs are as follows:
         * | INPUT         | OUTPUT    |
         * | ------------- | --------- |
         * | Kind::Numeric | "numeric" |
         * | Kind::Boolean | "boolean" |
         * | Kind::BitSet  | "bitset"  |
         * | Kind::String  | "string"  |
         */
        [[nodiscard]] static std::string_view constexpr name_of( Kind v ) noexcept;

        [[nodiscard]] static std::string_view constexpr name_of( PropertySpec v ) noexcept
        {
            return name_of( v.kind() );
        }

        /** @brief Convert raw 2-bit value to @ref PropertySpec::Kind. */
        /**
        * @details
        * Converts the given raw value (lower 2 bits) into a corresponding
        * @ref PropertySpec::Kind value.
        *
        * The input value is masked with @ref KIND_MASK to ensure that
        * only the valid resolution bits are used.
        *
        * @par Input / Output
        * | raw (uint8_t) | masked | Resulting Kind |
        * | ------------- | ------ | -------------- |
        * | 0b'0000'0000  | 0b'00  | Kind::Numeric  |
        * | 0b'0000'0001  | 0b'01  | Kind::Boolean  |
        * | 0b'0000'0010  | 0b'10  | Kind::BitSet   |
        * | 0b'0000'0011  | 0b'11  | Kind::String   |
        *
        * @note ja: 下位2ビットを @ref PropertySpec::Kind に変換します。
        *           入力値は @ref KIND_MASK によりマスクされます。
        *
        * @param raw The raw 2-bit encoded property spec kind value.
        * @return The corresponding @ref PropertySpec::Kind.
        */
        [[nodiscard]] static Kind constexpr from_raw( std::uint8_t raw ) noexcept
        {
            std::uint8_t const v = raw & KIND_MASK;
            return static_cast<Kind>( v );
        }

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
        [[nodiscard]] std::string str() const noexcept
        {
            std::ostringstream oss;

            oss << "{ code: "       << code_
                << ", kind: "       << name_of( kind() )
                << ", permission: " << Permission::name_of( permission() )
                << ", resolution: " << Resolution::name_of( resolution() )
                << ", init: "       << initVal_.str()
                << ", min: "        << minVal_.str()
                << ", max: "        << maxVal_.str()
                << " }";

            return oss.str();
        }

    private:

        static constexpr std::byte BOOL_MIN = std::byte{ 0x00 };
        static constexpr std::byte BOOL_MAX = std::byte{ 0x01 };

        [[nodiscard]] static Kind constexpr kind_of(
            value::Value255 const &min, value::Value255 const &max ) noexcept
        {
            auto min_size = min.size();
            auto max_size = max.size();

            if ( ( min_size == 0U ) && ( max_size == 0U ) )
            {
                return Kind::String;
            }

            if ( ( min_size == 0U ) && ( max_size != 0U ) )
            {
                return Kind::BitSet;
            }

            if ( ( min_size == 1U ) && ( min.bytes().at( 0U ) == BOOL_MIN ) &&
                 ( max_size == 1U ) && ( max.bytes().at( 0U ) == BOOL_MAX ) )
            {
                return Kind::Boolean;
            }

            return Kind::Numeric;
        }

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
            std::uint8_t perm: 2;
            std::uint8_t kind : 2;
            std::uint8_t reso: 3;
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

        [[nodiscard]] Kind kind() const noexcept
        {
            return from_raw( frags_.kind );
        }

        [[nodiscard]] Perm permission() const noexcept
        {
            return Permission::from_raw( frags_.perm );
        }

        [[nodiscard]] Reso resolution() const noexcept
        {
            return Resolution::from_raw( frags_.reso );
        }

        [[nodiscard]] value::Value255 const &initVal() const noexcept { return initVal_; }
        [[nodiscard]] value::Value255 const &minVal() const noexcept { return minVal_; }
        [[nodiscard]] value::Value255 const &maxVal() const noexcept { return maxVal_; }

        /* #endregion */

    private:

        /* #region : member variables */

        std::uint8_t code_;       // 1 bytes
        Fragments frags_;         // 1 bytes
        value::Value255 initVal_; // 6 bytes
        value::Value255 minVal_;  // 6 bytes
        value::Value255 maxVal_;  // 6 bytes
        // ---------------------------------
        //                   Total: 20 bytes

        /* #endregion */

    }; // class PropertySpec

    /** @brief Stream output operator for `PropertySpec::Kind`. */
    /**
     * @details
     * Outputs the string representation of the `PropertySpec::Kind` instance
     * to the provided output stream.
     *
     * @see PropertySpec::name_of() for the format of the output.
     *
     * @param os The output stream to write to.
     * @param v The `PropertySpec::Kind` instance to output.
     *
     * @return Reference to the output stream after writing.
     */
    std::ostream &operator<<( std::ostream &os, PropertySpec::Kind const &v ) noexcept;

    /** @brief Alias of the @ref PropertySpec::Kind */
    using PropKind = PropertySpec::Kind;

    /* ^\__________________________________________ */
    /* Static assertions.                           */
    static_assert( sizeof(machine::PropertySpec) == 20, "Unexpected PropertySpec size" );
    static_assert( alignof(machine::PropertySpec) == 1, "Unexpected PropertySpec alignment" );

} // namespace machine

namespace std {

    /** @brief Formatter specialization for `machine::PropertySpec::Kind`. */
    /**
     * @details
     * Formats a `Kind` instance. Examples are follows:
     * - `Kind::Numeric`: `numeric(0)`
     * - `Kind::Boolean`: `boolean(1)`
     * - `Kind::BitSet` : `bitset(2)`
     * - `Kind::String` : `string(3)`
     */
    template <>
    struct formatter<machine::PropertySpec::Kind>
    {
        /** @brief Parse format specifiers (none supported). */
        constexpr auto parse( std::format_parse_context &ctx )
        {
            return ctx.begin();
        }

        /** @brief Format `machine::PropertySpec::Kind` value. */
        template <typename FormatContext>
        auto format( machine::PropertySpec::Kind const &v, FormatContext &ctx ) const
        {
            return std::format_to( ctx.out(), "{}({})",
                machine::PropertySpec::name_of( v ), static_cast<int>( v ) );
        }
    };

} // namespace std

