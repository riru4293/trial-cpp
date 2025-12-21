/* Minimal task-based app_main
 * This file now contains a small FreeRTOS task that repeatedly calls
 * processing_loop(). The original example was removed per user request.
 */

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

// テストモードの場合はunityを実行
#ifdef TEST_BUILD
#include "unity.h"

void setUp(void) {}
void tearDown(void) {}

extern "C" void app_main(void)
{
    unity_run_menu();
}

#else
// 通常モード：通常のアプリケーション

static const char* TAG = "app_main";

// main processing method that runs inside the task's infinite loop
static void processing_loop()
{
    // Minimal work: log a message once per loop iteration.
    ESP_LOGI(TAG, "processing_loop iteration");
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
    auto task = new MainTask();
    if (!task) {
        ESP_LOGE(TAG, "Out of memory: cannot allocate MainTask");
        return;
    }

    if (!task->start()) {
        ESP_LOGE(TAG, "Failed to create main_task");
        delete task;
    }
}

#endif // TEST_BUILD
