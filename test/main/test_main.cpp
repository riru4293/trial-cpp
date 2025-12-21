#include <unity.h>
#include <esp_log.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <esp_system.h>

static const char *TAG = "test";

static void print_banner(const char* text)
{
    ESP_LOGI(TAG, "==== %s ====", text);
}

extern "C" void app_main(void)
{
    // 少し待機してシリアル接続が安定するのを待つ
    vTaskDelay(pdMS_TO_TICKS(1000));
    
    print_banner("Running all registered tests");
    UNITY_BEGIN();
    unity_run_all_tests();
    UNITY_END();
    
    ESP_LOGI(TAG, "All tests completed.");
    ESP_LOGI(TAG, "Restarting in 5 seconds...");
    vTaskDelay(pdMS_TO_TICKS(5000));
    esp_restart();
}
