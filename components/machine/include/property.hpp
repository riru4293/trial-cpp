#pragma once

/* C++ Standard Library */
#include <cstdint>
#include <ostream>
#include <string>

/* Custom Library */
#include <spec.hpp>
#include <value.hpp>

namespace machine
{
    class Property
    {
    /* ^\__________________________________________ */
    /* #region Static members, Inner types.         */

    public:

        /* #region SetResult */

        enum class SetResult : std::uint8_t
        {
            Success = 0,
            NoChange = 1,
            IllegalArgument = 2,
            Forbidden = 3,
            InternalError = 4,
        };

        /* #endregion */// SetResult

    /* ^\__________________________________________ */
    /* #region Constructors.                        */

    public:

        explicit Property( std::uint8_t code
                         , property::Spec &&spec
                         , property::MutableValue &&value ) noexcept
            : code_( code )
            , spec_( std::move( spec ) )
            , value_( std::move( value ) )
        { /* Do nothing */ }

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
