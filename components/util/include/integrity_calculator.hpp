/**
 * @file integrity_calculator.hpp
 * @brief Computes a integrity check data.
 * @note ja: 整合性チェックデータを計算します。
 */
#pragma once

#include <cstddef>
#include <cstdint>

namespace util {
    /** @brief Class for calculating integrity check data such as CRC. */
    /** @note ja: CRCなどの整合性チェックデータを計算するためのクラス。 */
    class IntegrityCalculator
    {
    public:
       /** @brief Calculates CRC-8 checksum for data using polynomial 0x07. */
       /** @note ja: 多項式 0x07 を使用してデータの CRC-8 チェックサムを計算します。 */
       /**
        * @details
        * This function computes an 8-bit cyclic redundancy check (CRC-8) for the
        * provided data using the standard CRC-8 algorithm with polynomial 0x07.
        * The computation is performed at compile-time if the arguments are constant expressions.
        * @note ja: この関数は、多項式0x07を使用する標準CRC-8アルゴリズムを使用して、
        *           指定されたデータに対して8ビット巡回冗長検査（CRC-8）を計算します。
        *           引数が定数式の場合、計算はコンパイル時に実行されます。
        */
       /**
        * @example
        * @code
        * // Sample data. The first 3 bytes are the payload,
        * // and the last byte (0xCC) is reserved for CRC-8.
        * // --
        * // ja: サンプルデータ。最初の3バイトはペイロードで、
        * //     最後のバイト（0xCC）はCRC-8のために予約されています。
        * std::byte data[] = { 0xA5, 0x01, 0x02, 0xCC };
        *
        * // Calculate the 3-byte payload portion (0xA5, 0x01, 0x02)
        * // and set CRC-8 to the last byte.
        * // --
        * // ja: 3バイトのペイロード部分（0xA5、0x01、0x02）を計算し、
        * //     CRC-8を最後のバイトに設定します。
        * data[3] = IntegrityCalculator::computeCrc8( data, 3U );
        * @endcode
        */
       /**
        * @param[in] data Pointer to the data buffer to calculate CRC-8 for.
        * @note ja: CRC-8を計算するデータバッファへのポインタ。
        * @param[in] size Number of bytes in the data buffer.
        * @note ja: データバッファ内のバイト数。
        *
        * @return The calculated CRC-8 checksum as a uint8_t value.
        * @note ja: 計算されたCRC-8チェックサムをuint8_t値として返します。
        */
        [[nodiscard]]
        static std::uint8_t constexpr computeCrc8(
            std::byte const *data, std::size_t size ) noexcept;
    };
}
