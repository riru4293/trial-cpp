/* Minimal task-based app_main
 * This file now contains a small FreeRTOS task that repeatedly calls
 * processing_loop(). The original example was removed per user request.
 */

#include "sdkconfig.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include <sstream>
#include <iomanip>
#include <format>

static const char* TAG = "app_main";

// main processing method that runs inside the task's infinite loop
static void processing_loop()
{
    // std::size_t counter = sizeof(machine::Property);
    // std::size_t counter = sizeof(unsigned long);

    // Minimal work: log a message once per loop iteration.
    ESP_LOGI(TAG, "processing_loop iteration");

    // machine::Unit iu = machine::Unit(machine::Unit::Kind::Internal, 0);
    // machine::Unit eu = machine::Unit(machine::Unit::Kind::External, 0);
    
    // std::ostringstream os;
    // os << "<<operator: iu=" << iu << " eu=" << eu;
    // std::string s1 = os.str();
    // ESP_LOGI(TAG, "%s", s1.c_str());

    // std::string s2 = std::format("std::format: iu={} eu={}", iu, eu);
    // ESP_LOGI(TAG, "%s", s2.c_str());
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
