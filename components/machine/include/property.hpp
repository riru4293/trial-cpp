#pragma once

/**
 * @file property.hpp
 * @brief Declaration of the `Property` class.
 *     @n （ja: `Property` クラスの宣言）
 *
 * @details
 * This header is declaration of the `Property` class.
 * The `Property` class represents a property of an external machine,
 * encapsulating its code, specification, and value.
 * @n @n ja: @n
 * このヘッダーは、`Property` クラスの宣言です。
 * `Property` クラスは、外部機械のプロパティを表し、そのコード、仕様、および値をカプセル化します。
 */

/* C++ Standard Library */
#include <cstdint>
#include <ostream>
#include <string>

/* Custom Library */
#include <spec.hpp>
#include <value.hpp>

namespace machine
{
    /** @brief Represents a property of an external machine.
     *      @n （ja: 外部機械のプロパティを表す） */
    /**
     * @details
     * This class encapsulates the code, specification, and value of a property
     * in an external machine. It provides mechanisms for construction,
     * string representation, and value updates with validation.
     * Only the value is mutable; the code and specification are immutable.
     * The specification includes the value's resolution, initial value,
     * minimum value, maximum value, and read/write permissions.
     * @n @n ja: @n
     * このクラスは、外部機械におけるプロパティのコード、仕様、および値をカプセル化します。
     * また、構築、文字列表現、および検証付きの値更新のメカニズムを提供します。
     * 値のみが変更可能であり、コードと仕様は不変です。
     * 仕様には、値のステップ数、初期値、最小値、最大値、読み書きの許可が含まれます。
     *
     * @par Thread Safety
     * This class is thread-safe. It performs mutual exclusion
     * on the only mutable member, the value.
     * @n @n ja: @n
     * このクラスはスレッドセーフです。唯一の変更可能なメンバーである値に対して
     * 排他制御を行います。
     */
    class Property
    {
    public:
        // ----- Factory methods -----

        // bytesは変化してはならない
        [[nodiscard]]
        static std::optional<Value255> create( std::span<std::byte const> bytes ) noexcept;
        // code : 1 byte *
        // length : 1 byte *
        // attribute kind: 1 byte
        //   - [0] r/w permission, step size : 1 + 1 + 1 bytes; if none, set to 0x00
        //   - [1] current value : 1 + 1+ N bytes; if none, set to 0x00
        //   - [2] initial value [For Read] : 1 + 1 + N bytes; if none, set to 0x00
        //   - [3] minimum value [For Write] : 1 + 1 + N bytes; if none, set to 0x00
        //   - [4] maximum value [For Write] : 1 + 1 + N bytes; if none, set to 0x00

        ~Property() noexcept = default;                 //!< Destructor (default).
        Property( const Property & ) noexcept = delete; //!< Copy constructor (deleted).
        Property( Property && ) noexcept = delete;      //!< Move constructor (deleted).

    /* #endregion */// Constructors

    /* ^\__________________________________________ */
    /* #region Operators.                           */

    public:

        Property &operator=( Property const & ) noexcept = delete;              //!< Copy operator (deleted).
        Property &operator=( Property && ) noexcept = delete;                   //!< Move operator (deleted).
        bool constexpr operator==( Property const & ) const noexcept = delete;  //!< Equality operator (deleted).
        auto constexpr operator<=>( Property const & ) const noexcept = delete; //!< Three-way comparison operator (deleted).

    /* #endregion */// Operators

    /* ^\__________________________________________ */
    /* #region Instance members.                    */

    public:

        /* #region Public methods */

        /** @brief Returns a string representation of the `Property`. */
        /**
         * @details
         * For example, a `Property` with code=`0xA5`, format=`Numeric`,
         * permission=`ReadWrite`, resolution=`X1`, initial_value=`10`,
         * minimum_value=`0`, maximum_value=`1024` and value=`999`
         * will be formatted as:
         * \code{.unparsed}
         * { code: 0xA5, spec: { format: numeric(0), permission: read-write(3),
         *   resolution: x1(0), initial_value: [ 0x0A ], minimum_value: [ 0x00 ],
         *   maximum_value: [ 0x00 0x04 ] }, value: [ 0xE7 0x03 ] }
         * \endcode
         *
         * @return String representation of the `Property`.
         */
        [[nodiscard]]
        std::string str() const noexcept;

        /* #endregion */// Public methods

        /* #region Getter methods */

        [[nodiscard]]
        std::uint8_t const &code() const noexcept { return code_; }

        [[nodiscard]]
        property::Spec const &spec() const noexcept { return spec_; }

        [[nodiscard]]
        property::Value const &value() const noexcept { return value_; }

        /* #endregion */// Getter methods

        /* #region Setter methods */

        [[nodiscard]]
        SetResult setValue( std::byte const *data, std::uint8_t size ) noexcept;

        /* #endregion */// Setter methods

    private:

        /* #region : Private methods */

        bool isWritable() noexcept;

        bool isValidValue( std::byte const *data, std::uint8_t size ) noexcept;

        SetResult updateValue( std::byte const *data, std::uint8_t size ) noexcept;

        /* #endregion */ // Private methods

        /* #region : member variables */

        std::uint8_t code_;            //  1 byte
        property::Spec spec_;          // 19 bytes
        property::MutableValue value_; //  6 bytes
        // -----------------------------------------------------
        //                         Total: 26 bytes

        /* #endregion */

    }; // class Property

    /** @brief Stream output operator for `Property`. */
    /**
     * @details
     * Outputs the string representation of the `Property` instance
     * to the provided output stream.
     *
     * @see Property::str() for the format of the output.
     *
     * @param os The output stream to write to.
     * @param v The `Property` instance to output.
     *
     * @return Reference to the output stream after writing.
     */
    std::ostream &operator<<( std::ostream &os, Property const &v ) noexcept;

    /* ^\__________________________________________ */
    /* Static assertions.                           */
    static_assert(  sizeof(Property) == 26U, "Unexpected Property size");
    static_assert( alignof(Property) == 1U,  "Unexpected Property alignment");

} // namespace machine
