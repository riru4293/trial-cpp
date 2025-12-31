#pragma once

/* C++ Standard Library */
#include <cstddef>
#include <cstdint>
#include <optional>
#include <ostream>
#include <string>

/* Custom Library */
#include <format.hpp>
#include <permission.hpp>
#include <resolution.hpp>
#include <value.hpp>

namespace machine::property
{

    /** @brief Represents a property specification. */
    /**
     * @details
     *
     * @par Overview:
     * A property specification consists of the following items:
     *
     * 1. Value format
     * 2. Value access permission
     * 3. Value resolution
     * 4. Initial value
     * 5. Minimum value
     * 6. Maximum value
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
     *
     * @see Format
     * @see Permission
     * @see Resolution
     * @see value::Value255
     */
    class Spec
    {
    /* ^\__________________________________________ */
    /* #region Static members, Inner types.         */

    public:

        /* #region Factory methods */

        /** @brief Create a Spec instance with given parameters. */
        /**
         * @param permission value access permission
         * @param resolution value resolution
         * @param init_val Pointer to the initial property value
         *                 A null pointer is only valid if size is 0.
         * @param init_size size of initial property value
         * @param min_val Pointer to the minimum property value
         *                A null pointer is only valid if size is 0.
         * @param min_size size of minimum property value
         * @param max_val Pointer to the maximum property value
         *                A null pointer is only valid if size is 0.
         * @param max_size size of maximum property value
         *
         * @return Spec instance if parameters are valid; std::nullopt otherwise.
         */
        static std::optional<Spec> create( Permission::Kind permission
                                         , Resolution::Kind resolution
                                         , std::byte const *init_val
                                         , std::uint8_t init_size
                                         , std::byte const *min_val
                                         , std::uint8_t min_size
                                         , std::byte const *max_val
                                         , std::uint8_t max_size ) noexcept;
        
        /** @brief Create a Spec instance with given parameters. */
        /**
         * @param permission value access permission
         * @param resolution value resolution
         * @param init_val initial property value
         * @param min_val minimum property value
         * @param max_val maximum property value
         *
         * @return Spec instance if parameters are valid; std::nullopt otherwise.
         */
        static std::optional<Spec> create( Permission::Kind permission
                                         , Resolution::Kind resolution
                                         , Value const &init_val
                                         , Value const &min_val
                                         , Value const &max_val ) noexcept;
        
    private:

        static std::optional<Spec> create( Permission::Kind permission
                                         , Resolution::Kind resolution
                                         , std::optional<Value> &&init
                                         , std::optional<Value> &&min
                                         , std::optional<Value> &&max ) noexcept;

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

        ~Spec() noexcept = default;             //!< Destructor (default).
        Spec( const Spec & ) noexcept = delete; //!< Copy constructor (deleted).
        Spec( Spec && ) noexcept = default;     //!< Move constructor (default).

    private:

        explicit Spec( Fragments frags
                     , Value &&init_val
                     , Value &&min_val
                     , Value &&max_val ) noexcept;

    /* #endregion */// Constructors

    /* ^\__________________________________________ */
    /* #region Operators.                           */

    public:

        Spec &operator=( Spec const & ) noexcept = delete;          //!< Copy operator (deleted).
        Spec &operator=( Spec && ) noexcept = default;              //!< Move operator (default).
        bool constexpr operator==( Spec const & ) const noexcept = delete;  //!< Equality operator (deleted).
        auto constexpr operator<=>( Spec const & ) const noexcept = delete; //!< Three-way comparison operator (deleted).

    /* #endregion */// Operators

    /* ^\__________________________________________ */
    /* #region Instance members.                    */

    public:

        /* #region Public methods */

        /** @brief Checks if the given value is within the range specified by the `Spec`. */
        /**
         * @details
         * This method checks whether the provided value falls within the
         * minimum and maximum values defined in the `Spec`.
         *
         * @param v [in] The value to check.
         *
         * @return `true` if the value is within range; `false` otherwise.
         */
        [[nodiscard]]
        bool isWithinRange( Value const &v ) const noexcept;

        /** @brief Returns a string representation of the `Spec`. */
        /**
         * @details
         * For example, a `Spec` with format=`Numeric`, permission=`ReadWrite`,
         * resolution=`X1`, initial_value=`10`, minimum_value=`0`, maximum_value=`1024`
         * will be formatted as:
         * @verbatim
           { format: numeric(0), permission: read-write(3), resolution: x1(0),
             initial_value: [ 0x0A ], minimum_value: [ 0x00 ], maximum_value: [ 0x00 0x04 ] }
           @endverbatim
         *
         * @return String representation of the `Spec`.
         */
        [[nodiscard]]
        std::string str() const noexcept;

        /* #endregion */// Public methods

        /* #region Getter methods */

        [[nodiscard]]
        Format::Kind format() const noexcept
        {
            return Format::fromRaw( frags_.format );
        }

        [[nodiscard]]
        Permission::Kind permission() const noexcept
        {
            return Permission::fromRaw( frags_.permission );
        }

        [[nodiscard]]
        Resolution::Kind resolution() const noexcept
        {
            return Resolution::fromRaw( frags_.resolution );
        }

        [[nodiscard]]
        Value const &initVal() const noexcept { return initVal_; }

        [[nodiscard]]
        Value const &minVal() const noexcept { return minVal_; }

        [[nodiscard]]
        Value const &maxVal() const noexcept { return maxVal_; }

        /* #endregion */// Getter methods

    private:

        /* #region : Private methods */

        std::int32_t decodeNumericValue( Value const &v ) const noexcept;

        /* #endregion */ // Private methods

        /* #region : member variables */

        Fragments frags_;   // 1 bytes
        Value initVal_;     // 6 bytes
        Value minVal_;      // 6 bytes
        Value maxVal_;      // 6 bytes
        // ---------------------------------
        //             Total: 19 bytes

        /* #endregion */

    }; // class Spec

    /** @brief Stream output operator for `Spec`. */
    /**
     * @details
     * Outputs the string representation of the `Spec` instance
     * to the provided output stream.
     *
     * @see Spec::str() for the format of the output.
     *
     * @param os The output stream to write to.
     * @param v The `Spec` instance to output.
     *
     * @return Reference to the output stream after writing.
     */
    std::ostream &operator<<( std::ostream &os, Spec const &v ) noexcept;

    /* ^\__________________________________________ */
    /* Static assertions.                           */
    static_assert( sizeof(machine::property::Spec) == 19, "Unexpected Spec size" );
    static_assert( alignof(machine::property::Spec) == 1, "Unexpected Spec alignment" );

} // namespace machine
