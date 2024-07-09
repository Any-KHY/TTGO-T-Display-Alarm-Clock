/* wifi setting from TTGO demo */
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
//#include <driver/touch_pad.h>
#include "esp_wpa2.h"

#include "input_output.h"
#include "networking.h"


typedef void (* wifi_tx_done_cb_t)(uint8_t ifidx, uint8_t *data, uint16_t *data_len, bool txStatus);
esp_err_t esp_wifi_set_tx_done_cb(wifi_tx_done_cb_t cb);

wifi_mode_type wifi_mode=0;

int wifi_connected() {
    if(network_event_group)
        return xEventGroupGetBits(network_event_group) & CONNECTED_BIT;
    return 0;
}
void init_wifi(wifi_mode_type mode) {
    if(wifi_mode==mode && network_interface!=NULL &&
            (xEventGroupGetBits(network_event_group) & CONNECTED_BIT))
        return;
    if(network_event_group==NULL)
        network_event_group = xEventGroupCreate();
    xEventGroupClearBits(network_event_group, AUTH_FAIL | CONNECTED_BIT);
    wifi_mode=mode;
    if(network_interface!=NULL) {
        esp_wifi_stop();
        esp_wifi_deinit();
        esp_event_loop_delete_default();
        esp_wifi_clear_default_wifi_driver_and_handlers(network_interface);
        esp_netif_destroy(network_interface);
        esp_netif_deinit();
       // network_interface=NULL;
    }
    esp_netif_init();
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    if(mode==ACCESS_POINT)
        network_interface = esp_netif_create_default_wifi_ap();
    else
        network_interface = esp_netif_create_default_wifi_sta();
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, &event_handler, NULL) );
    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));

    uint8_t protocol=(WIFI_PROTOCOL_11B|WIFI_PROTOCOL_11G|WIFI_PROTOCOL_11N);//|WIFI_PROTOCOL_LR);
    if(mode==ACCESS_POINT) {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_AP));
        esp_wifi_set_protocol(ESP_IF_WIFI_AP,protocol);
        #define SSID "ESP32"
        wifi_config_t wifi_config = { .ap = {
                .ssid = SSID,
                .ssid_len = strlen(SSID),
                .channel = 3,
                .password = "",
                .max_connection = 8,
                .authmode = WIFI_AUTH_OPEN
            },
        };
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_config)); 
    } else {
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        esp_wifi_set_protocol(ESP_IF_WIFI_STA,protocol);
        char ssid[32];
        storage_read_string("ssid","MasseyWifi",ssid,sizeof(ssid));
        char password[64];
        storage_read_string("password","", password, sizeof(password));
        char username[64];
        storage_read_string("username","", username, sizeof(username));
        wifi_config_t wifi_config = {0};
        strncpy((char *)wifi_config.sta.ssid,ssid,sizeof(wifi_config.sta.ssid));
        strncpy((char *)wifi_config.sta.password,password,sizeof(wifi_config.sta.password));
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));
        if(strlen(username)!=0) {
            ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_username((uint8_t *)username, strlen(username)) );
            ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_set_password((uint8_t *)password, strlen(password)) );
            ESP_ERROR_CHECK( esp_wifi_sta_wpa2_ent_enable() );
        }
    }

    ESP_ERROR_CHECK(esp_wifi_start());
}


void wifi_connect(int onlyconnect) {
    cls(0);
    network_event[0]=0;
    init_wifi(STATION);
    do {
        if(xEventGroupGetBits(network_event_group) & CONNECTED_BIT) {
            wifi_ap_record_t ap;
            //esp_netif_ip_info_t ip_info;
            if(onlyconnect) {
                flip_frame();
                return;
            }
            esp_wifi_sta_get_ap_info(&ap);
        }
        if(xEventGroupGetBits(network_event_group) & AUTH_FAIL) {
            setFont(FONT_UBUNTU16);
            setFontColour(255,255,255);
            gprintf("Authentication Failed\n");
        }
        flip_frame();
    } while(get_input()!=RIGHT_DOWN);
}
