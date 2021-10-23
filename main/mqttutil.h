#include "mqtt_client.h"

int mqtt_send(esp_mqtt_client_handle_t client, int value);
esp_mqtt_client_handle_t mqtt_setup(const esp_mqtt_client_config_t* mqtt_cfg);
