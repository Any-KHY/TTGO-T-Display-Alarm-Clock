/* network setting from TTGO demo */

#include <esp_system.h>
#include <esp_wifi.h>
#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/task.h>
#include <driver/gpio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <esp_log.h>
#include <esp_sntp.h>
#include <nvs_flash.h>
#include <fonts.h>
#include <graphics.h>
#include <time.h>
#include "esp_smartconfig.h"

#include "lwip/sockets.h"
#include <esp_http_server.h>
#include "mqtt_client.h"

#include "FreeSansBold18pt7b.h"
#include "FreeSansBold24pt7b.h"
#include <driver/touch_pad.h>
#include "esp_wpa2.h"

#include "input_output.h"
#include "networking.h"
#include "esp_http_client.h"
#include "esp32/rom/tjpgd.h"

EventGroupHandle_t network_event_group;
char network_event[64];
//#define TAG "Networking"
int bg_col=0;
esp_netif_t *network_interface = NULL;

void set_event_message(const char *s) {
    snprintf(network_event,sizeof(network_event),"%s\n",s);
}

mqtt_callback_type mqtt_callback=0;

void set_mqtt_callback(mqtt_callback_type callback) {
    mqtt_callback=callback;
}

void event_handler(void *arg, esp_event_base_t event_base,
                                int32_t event_id, void* event_data) {
    if (event_base == WIFI_EVENT) {
        //set_event_message(wifi_messages[event_id%ARRAY_LENGTH(wifi_messages)]);
        wifi_event_sta_disconnected_t* disconnect_data;
        switch (event_id) {
        case WIFI_EVENT_STA_START:
            xEventGroupClearBits(network_event_group, AUTH_FAIL | CONNECTED_BIT);
            if (wifi_mode == STATION)
                esp_wifi_connect();
            break;
        case WIFI_EVENT_STA_DISCONNECTED:
            disconnect_data=event_data;
            xEventGroupClearBits(network_event_group, CONNECTED_BIT);
            if(disconnect_data->reason==WIFI_REASON_AUTH_FAIL ||
                disconnect_data->reason==WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT)
                xEventGroupSetBits(network_event_group, AUTH_FAIL);
            break;
        case WIFI_EVENT_SCAN_DONE:
            esp_wifi_scan_start(NULL, false);
            break;
        }
    }
    if (event_base == IP_EVENT) {
        if ((event_id == IP_EVENT_STA_GOT_IP) ||
            (event_id == IP_EVENT_AP_STAIPASSIGNED) || (event_id == IP_EVENT_ETH_GOT_IP)) {
            xEventGroupSetBits(network_event_group, CONNECTED_BIT);
        }
    }
    if (!strcmp(event_base,"MQTT_EVENTS")) {
        if(mqtt_callback) mqtt_callback(event_id,event_data);

    }
}

esp_mqtt_client_handle_t mqtt_client = NULL;

void mqtt_connect(mqtt_callback_type callback) {
    char client_name[32];
    if(mqtt_client!=NULL) mqtt_disconnect();
    srand(esp_timer_get_time());
    esp_mqtt_client_config_t mqtt_cfg = { .broker.address.uri = "mqtt://mqtt.webhop.org",.credentials.client_id=client_name};
    wifi_connect(1); // connect to Massey Wifi
    if(xEventGroupGetBits(network_event_group) & CONNECTED_BIT) {
        mqtt_client=esp_mqtt_client_init(&mqtt_cfg);
        esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, event_handler, NULL);
        set_mqtt_callback(callback);
        esp_mqtt_client_start(mqtt_client);
    }
}

void mqtt_disconnect() {
    if(mqtt_client!=NULL) {
        esp_mqtt_client_stop(mqtt_client);
        esp_mqtt_client_destroy(mqtt_client);
        mqtt_client=NULL;
        mqtt_callback=NULL;
    }
}
