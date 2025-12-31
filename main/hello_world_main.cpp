/* Minimal task-based app_main
 * This file now contains a small FreeRTOS task that repeatedly calls
 * processing_loop(). The original example was removed per user request.
 */

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <format.hpp>
#include <permission.hpp>
#include <resolution.hpp>
#include <spec.hpp>
#include <value.hpp>
#include <optional>
#include <bit>

static const char* TAG = "app_main";

// main processing method that runs inside the task's infinite loop
static void processing_loop()
{
    // Minimal work: log a message once per loop iteration.
    ESP_LOGI(TAG, "processing_loop iteration");

    using namespace machine::property;

    using Array1 = std::array<std::byte, 1U>;
    using Array4 = std::array<std::byte, 4U>;
    {
        Array1 max = std::bit_cast<Array1>( static_cast<uint8_t>( 203U ) );
        Array1 init = std::bit_cast<Array1>( static_cast<uint8_t>( 3U ) );

        std::optional<Spec> spec =
            Spec::create( Permission::Kind::ReadWrite
                        , Resolution::Kind::X1
                        , init.data(), init.size()
                        , nullptr, 0U
                        , max.data() , max.size() );
        
        if ( spec.has_value() )
        {
            ESP_LOGI( TAG, "Spec created: %s", spec->str().data() );
        }
        else
        {
            ESP_LOGE( TAG, "Failed to create Spec" );
        }
    }

    {
        Array4 min = std::bit_cast<Array4>( INT32_MIN );
        Array4 max = std::bit_cast<Array4>( INT32_MAX );
        Array4 init = std::bit_cast<Array4>( -1 );

        std::optional<Spec> spec =
            Spec::create( Permission::Kind::ReadWrite
                        , Resolution::Kind::X1
                        , init.data(), init.size()
                        , min.data() , min.size()
                        , max.data() , max.size() );
        
        if ( spec.has_value() )
        {
            ESP_LOGI( TAG, "Spec created: %s", spec->str().data() );
        }
        else
        {
            ESP_LOGE( TAG, "Failed to create Spec" );
        }
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

    static constexpr size_t stackSize_ = 4096;
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
