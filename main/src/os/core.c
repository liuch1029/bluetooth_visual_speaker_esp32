/*
 * core.c
 *
 *  Created on: 2018-03-04 20:07
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include "esp_log.h"
#include "esp_sleep.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#define TAG "os_core"

EventGroupHandle_t user_event_group;

#ifdef CONFIG_ENABLE_WAKEUP_KEY
void os_enter_sleep_mode(void)
{
    ESP_LOGI(TAG, "entering sleep mode");
#ifdef CONFIG_WAKEUP_KEY_MODE_LOW
    esp_sleep_enable_ext1_wakeup(1ULL << CONFIG_WAKEUP_KEY_PIN, ESP_EXT1_WAKEUP_ALL_LOW);
#else
    esp_sleep_enable_ext1_wakeup(1ULL << CONFIG_WAKEUP_KEY_PIN, ESP_EXT1_WAKEUP_ANY_HIGH);
#endif
    esp_deep_sleep_start();
}

static void os_get_wakeup_cause(void)
{
    portTickType xLastWakeTime = xTaskGetTickCount();

    if (esp_sleep_get_wakeup_cause() != ESP_SLEEP_WAKEUP_UNDEFINED) {
        vTaskDelayUntil(&xLastWakeTime, CONFIG_WAKEUP_KEY_HOLD_TIME / portTICK_RATE_MS);

#ifdef CONFIG_WAKEUP_KEY_MODE_LOW
        if (gpio_get_level(CONFIG_WAKEUP_KEY_PIN) == 0) {
#else
        if (gpio_get_level(CONFIG_WAKEUP_KEY_PIN) == 1) {
#endif
            ESP_LOGI(TAG, "wakeup from sleep mode");

            return;
        } else {
            ESP_LOGI(TAG, "wakeup aborted");

            os_enter_sleep_mode();
        }
    } else {
        os_enter_sleep_mode();
    }
}
#endif // CONFIG_ENABLE_WAKEUP_KEY

void core_init(void)
{
#ifdef CONFIG_ENABLE_WAKEUP_KEY
    os_get_wakeup_cause();
#endif
    user_event_group = xEventGroupCreate();
}
