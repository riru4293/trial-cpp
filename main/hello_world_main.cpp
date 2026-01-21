/* Minimal task-based app_main
 * This file now contains a small FreeRTOS task that repeatedly calls
 * processing_loop(). The original example was removed per user request.
 */

#include "property.hpp"
#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <cstddef>
#include <format.hpp>
#include <permission.hpp>
#include <resolution.hpp>
#include <spec.hpp>
#include <value.hpp>
#include <optional>
#include <bit>
#include <span>
#include <array>
#include <cstdint>
#include <cstring>
#include <string>
#include <unordered_map>

static const char* TAG = "app_main";

static void logSpec( std::optional<machine::property::Spec> const &spec )
{
    if ( spec.has_value() )
    {
        ESP_LOGI( TAG, "Spec created: %s", spec->str().data() );
    }
    else
    {
        ESP_LOGE( TAG, "Failed to create Spec" );
    }
}

/**
 * @brief Calculates CRC-8 checksum for data using polynomial 0x07
 *
 * This function computes an 8-bit cyclic redundancy check (CRC-8) for the
 * provided data using the standard CRC-8 algorithm with polynomial 0x07.
 * The computation is performed at compile-time if the arguments are constant expressions.
 *
 * @param[in] data Pointer to the data buffer to calculate CRC-8 for
 * @param[in] size Number of bytes in the data buffer
 *
 * @return The calculated CRC-8 checksum as a uint8_t value
 *
 * @note This function is constexpr and noexcept, allowing it to be used in
 *       compile-time evaluations with no exception safety implications.
 *
 * @example
 * @code
 * std::byte buffer[] = {0xA5, 0x01, 0x02};
 * std::uint8_t checksum = crc8(buffer, sizeof(buffer));
 * @endcode
 */
constexpr std::uint8_t crc8(const std::byte* data, std::size_t size) noexcept
{
    std::uint8_t crc = 0x00;

    for (std::size_t i = 0; i < size; ++i) {
        crc ^= static_cast<std::uint8_t>(data[i]);

        for (int b = 0; b < 8; ++b) {
            if (crc & 0x80) {
                crc = static_cast<std::uint8_t>((crc << 1) ^ 0x07);
            } else {
                crc <<= 1;
            }
        }
    }
    return crc;
}

// Property element definition
enum class Element : std::uint8_t
{
    Unknown       = 0x00,
    Permission    = 0x01,
    Resolution    = 0x02,
    InitialValue  = 0x03,
    MinimumValue  = 0x04,
    MaximumValue  = 0x05,
    CurrentValue  = 0x06,
};

static Element fromRaw( std::uint8_t raw ) noexcept
{
    switch ( raw )
    {
        case 0x01: return Element::Permission;
        case 0x02: return Element::Resolution;
        case 0x03: return Element::InitialValue;
        case 0x04: return Element::MinimumValue;
        case 0x05: return Element::MaximumValue;
        case 0x06: return Element::CurrentValue;
        default:   return Element::Unknown; // Default fallback
    }
}



// main processing method that runs inside the task's infinite loop
static void processing_loop()
{
    // Minimal work: log a message once per loop iteration.
    ESP_LOGI(TAG, "processing_loop iteration");

#   if 1
    using namespace machine::property;

    using Array1 = std::array<std::byte, 1U>;
    using Array4 = std::array<std::byte, 4U>;


    { // BitSet example
        Array1 max = std::bit_cast<Array1>( static_cast<std::uint8_t>( 203U ) );
        Array1 init = std::bit_cast<Array1>( static_cast<std::uint8_t>( 3U ) );

        std::optional<Spec> spec =
            Spec::create( static_cast<std::uint8_t>( Permission::Kind::ReadOnly )
                        , Resolution::NONE
                        , init.data(), init.size()
                        , nullptr, 0U
                        , max.data() , max.size() );

        logSpec( spec );
    }

    { // Numeric example#1
        Array1 min = std::bit_cast<Array1>( static_cast<int8_t>( 12 ) );
        Array1 max = std::bit_cast<Array1>( static_cast<int8_t>( 70 ) );
        Array1 init = std::bit_cast<Array1>( static_cast<int8_t>( 33 ) );

        logSpec( Spec::create(
            static_cast<std::uint8_t>( Permission::Kind::WriteOnly )
          , static_cast<std::uint8_t>( Resolution::Kind::X0_5 )
          , init.data(), init.size()
          , min.data() , min.size()
          , max.data() , max.size() ) );
    }

    { // String example

        std::string init_value = "Hello, World!";

        auto init = std::as_bytes( std::span{ init_value } );

        logSpec( Spec::create(
            static_cast<std::uint8_t>( Permission::Kind::ReadWrite )
          , Resolution::NONE
          , init.data(), static_cast<std::uint8_t>( init.size() )
          , nullptr, 0U
          , nullptr, 0U ) );
    }

    { // Boolean example
        logSpec( Spec::create(
            static_cast<std::uint8_t>( Permission::Kind::ReadWrite )
          , Resolution::NONE
          , &detail::BOOL_FALSE, detail::BOOL_SIZE
          , &detail::BOOL_FALSE, detail::BOOL_SIZE
          , &detail::BOOL_TRUE, detail::BOOL_SIZE ) );
    }

    { // Numeric example#2
        Array4 min = std::bit_cast<Array4>( INT32_MIN );
        Array4 max = std::bit_cast<Array4>( INT32_MAX );
        Array4 init = std::bit_cast<Array4>( -1 );

        logSpec( Spec::create(
            static_cast<std::uint8_t>( Permission::Kind::ReadWrite )
          , static_cast<std::uint8_t>( Resolution::Kind::X0_01 )
          , init.data(), init.size()
          , min.data() , min.size()
          , max.data() , max.size() ) );
    }
#   endif
                       // Code
    std::byte data[] = { std::byte{0xA5}
                       // Element count
                       , std::byte{0x06}
                       // -------------------------------------
                       // Element#1 id
                       , std::byte{static_cast<std::uint8_t>( Element::Permission )}
                       // Element#1 size
                       , std::byte{0x01}
                       // Element#1 values
                       , std::byte{0x02} // 0x02=ReadWrite
                       // -------------------------------------
                       // Element#2 id
                       , std::byte{static_cast<std::uint8_t>( Element::Resolution )}
                       // Element#2 size
                       , std::byte{0x01}
                       // Element#2 values
                       , std::byte{0x01} // 0x01=X5
                       // -------------------------------------
                       // Element#3 id
                       , std::byte{static_cast<std::uint8_t>( Element::InitialValue )}
                       // Element#3 size
                       , std::byte{0x02} // 2 byte
                       // Element#3 values
                       , std::byte{0x01} // 0x0101=257
                       , std::byte{0x01} // ------
                       // -------------------------------------
                       // Element#4 id
                       , std::byte{static_cast<std::uint8_t>( Element::MinimumValue )}
                       // Element#4 size
                       , std::byte{0x01} // 1 byte
                       // Element#4 values
                       , std::byte{0x01} // 0x01=1
                       // -------------------------------------
                       // Element#5 id
                       , std::byte{static_cast<std::uint8_t>( Element::MaximumValue )}
                       // Element#5 size
                       , std::byte{0x04} // 4 byte
                       // Element#5 values
                       , std::byte{0xFF} // 0x7FFFFFFF=2147483647
                       , std::byte{0xFF} //
                       , std::byte{0xFF} //
                       , std::byte{0x7F} // ---------------------
                       // -------------------------------------
                       // Element#6 id
                       , std::byte{static_cast<std::uint8_t>( Element::CurrentValue )}
                       // Element#6 size
                       , std::byte{0x03} // 3 byte
                       // Element#6 values
                       , std::byte{0xA5} // 0xA5A5A5=1094795589
                       , std::byte{0xA5} //
                       , std::byte{0xA5} // ---------------------
                       // -------------------------------------
                       , std::byte{0xCC} };


    // Calculate and append CRC8
    std::size_t constexpr payload_size = sizeof( data ) - 1U;
    data[payload_size] = std::byte{ crc8( data, payload_size ) };

    // Dump data with CRC8 appended
    ESP_LOGI( TAG, "Data with CRC8 appended:" );

    for ( std::size_t i = 0; i < payload_size; i++ )
    {
        ESP_LOGI( TAG, "  Byte[%02zu]: 0x%02X"
                 , i
                 , static_cast<unsigned>( data[i] ) );
    }

    ESP_LOGI( TAG, "  CRC8: 0x%02X"
             , static_cast<unsigned>( data[ payload_size ] ) );

    // Storage for element values
    std::unordered_map<Element, std::vector<std::byte>> element_values;

    // Initialize element values
    element_values.clear();
    for ( std::uint8_t i = static_cast<std::uint8_t>( Element::Permission );
          i <= static_cast<std::uint8_t>( Element::CurrentValue );
          i++ )
    {
        Element element_kind = fromRaw( i );
        element_values[ element_kind ] = std::vector<std::byte>{};
    }

    // ~~~~~~~~~~~~~~~~~~~~
    // ==== Parse data ====
    std::uint8_t idx = 0U;

    // Read property code
    std::uint8_t property_code = static_cast<std::uint8_t>( data[idx++] );

    // Read property element count
    std::uint8_t element_count = static_cast<std::uint8_t>( data[idx++] );

    // Read each element
    while ( element_count-- )
    {
        // Read element id and resolve it kind
        std::uint8_t element_id = static_cast<std::uint8_t>( data[idx++] );
        Element element_kind = fromRaw( element_id );

        // Read element size
        std::uint8_t element_size = static_cast<std::uint8_t>( data[idx++] );

        // Read element value(s)
        std::vector<std::byte> element_value( data + idx, data + idx + element_size );

        // Store element value
        if( element_kind != Element::Unknown )
            element_values[ element_kind ] = element_value;

        // Log element info
        ESP_LOGI( TAG, "  Element ID: %u, Size: %u", element_id, element_size );

        for ( auto const &b : element_value )
        {
            ESP_LOGI( TAG, "    Value byte: 0x%02X", static_cast<unsigned>( b ) );
        }

        idx += element_size;
    }

    // Permission value
    std::uint8_t permission = element_values.at( Element::Permission ).size() > 0
        ? static_cast<std::uint8_t>( element_values.at( Element::Permission )[0] )
        : 0xFFU;

    // Resolution value
    std::uint8_t resolution = element_values.at( Element::Resolution ).size() > 0
        ? static_cast<std::uint8_t>( element_values.at( Element::Resolution )[0] )
        : 0xFFU;

    // Create Property Spec
    std::optional<machine::property::Spec> property_spec = machine::property::Spec::create(
          permission
        , resolution
        , element_values.at( Element::InitialValue ).data()
        , static_cast<std::uint8_t>( element_values.at( Element::InitialValue ).size() )
        , element_values.at( Element::MinimumValue ).data()
        , static_cast<std::uint8_t>( element_values.at( Element::MinimumValue ).size() )
        , element_values.at( Element::MaximumValue ).data()
        , static_cast<std::uint8_t>( element_values.at( Element::MaximumValue ).size() ) );

    // Create Property Value
    std::optional<machine::property::MutableValue> current_value =
        machine::property::MutableValue::create(
            element_values.at( Element::CurrentValue ).data()
          , static_cast<std::uint8_t>( element_values.at( Element::CurrentValue ).size() ) );

    // Create Property
    if ( property_spec.has_value() && current_value.has_value() )
    {
        machine::Property property{
            property_code,
            std::move( property_spec.value() ),
            std::move( current_value.value() )
        };
        ESP_LOGI(TAG, "Property created with code: %u", property_code);
        ESP_LOGI(TAG, "Property spec: %s", property.spec().str().data() );
        ESP_LOGI(TAG, "Property value: %s", property.value().str().data() );
    }
    else
    {
        ESP_LOGE(TAG, "Failed to create Property with code: %u", property_code);
    }

    // CRC9
    std::byte received_crc = data[ idx ];
    std::byte calculated_crc = std::byte{ crc8( data, idx ) };

    if ( received_crc == calculated_crc )
    {
        ESP_LOGI( TAG, "CRC8 check passed: 0x%02X"
                 , static_cast<unsigned>( received_crc ) );
    }
    else
    {
        ESP_LOGE( TAG, "CRC8 check failed: received 0x%02X, calculated 0x%02X"
                 , static_cast<unsigned>( received_crc )
                 , static_cast<unsigned>( calculated_crc ) );
    }
}

// C++ class that owns the task and runs the processing loop
class MainTask {
public:
    MainTask() = default;

    // start the FreeRTOS task
    bool start() {
        BaseType_t res = xTaskCreate(
            &MainTask::taskEntry,
            "main_task",
            stackSize_,
            this,
            tskIDLE_PRIORITY + 1,
            &handle_);

        return res == pdPASS;
    }

    // the method that runs repeatedly inside the task
    void run() {
        while (true) {
            processing_loop();
            vTaskDelay(pdMS_TO_TICKS(1000));
        }
    }

    // optional: stop would delete/notify, not used in embedded example
    void stop() {
        if (handle_) {
            vTaskDelete(handle_);
            handle_ = nullptr;
        }
    }

private:
    static void taskEntry(void* param) {
        auto self = static_cast<MainTask*>(param);
        if (self) self->run();
        // Never returns
        vTaskDelete(nullptr);
    }

    static constexpr std::size_t stackSize_ = 4096;
    TaskHandle_t handle_ = nullptr;
};

extern "C" void app_main()
{
    // Mutex::create(); // Example usage of Mutex class

    auto task = new MainTask();
    if (!task) {
        ESP_LOGE(TAG, "Out of memory: cannot allocate MainTask");
        return;
    }

    if (!task->start()) {
        ESP_LOGE(TAG, "Failed to create main_task");
        // cleanup if needed
        delete task;
    }
    // intentionally not deleting `task` here — typical embedded apps keep it alive
}
