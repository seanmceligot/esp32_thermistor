#include <stdio.h>
#include <stdlib.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "mqttutil.h"
#include "wifiutil.h"
#include "thermistor.h"

#define CONFIG_BROKER_URL "mqtt://192.168.1.161"
static const char *TAG = "Thermistor";

void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ESP_LOGI(TAG, "IP_EVENT_STA_GOT_IP");
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        ESP_LOGI(TAG, "got ip:" IPSTR "\n", IP2STR(&event->ip_info.ip));

        ESP_LOGI(TAG, "start mqtt to: %s", CONFIG_BROKER_URL);
        esp_mqtt_client_config_t mqtt_cfg = {
            .uri = CONFIG_BROKER_URL,
        };
        esp_mqtt_client_handle_t mqtt_client = mqtt_setup(&mqtt_cfg);

        bool cali_enable = setup_adc();
        
        while (1) {
          int value = read_adc(cali_enable);
          mqtt_send(mqtt_client, value); 
          vTaskDelay(pdMS_TO_TICKS(1000));
        }
    } else {
        ESP_LOGI(TAG, "unknown event_base %s", event_base);
    }
}

void app_main(void) {
    start_wifi(&ip_event_handler);
}
