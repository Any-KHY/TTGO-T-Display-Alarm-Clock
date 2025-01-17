/* network setting from TTGO demo */

#include <freertos/event_groups.h>
#include <esp_wifi.h>

typedef enum
{
    SCAN,
    STATION,
    ACCESS_POINT,
} wifi_mode_type;

#define ARRAY_LENGTH(array) (sizeof((array)) / sizeof((array)[0]))

extern EventGroupHandle_t network_event_group;
void init_eth();
extern wifi_mode_type wifi_mode;
#define CONNECTED_BIT 1
#define AUTH_FAIL 2
extern esp_netif_t *network_interface;
extern char network_event[64];
extern int bg_col;

typedef void (*mqtt_callback_type)(int event_id, void *event_data);
void set_mqtt_callback(mqtt_callback_type callback);

void mqtt_connect(mqtt_callback_type callback);
void mqtt_disconnect();

void event_handler(void *arg, esp_event_base_t event_base,
                   int32_t event_id, void *event_data);
//void wifi_disconnect();
void wifi_connect(int onlyconnect);
void init_wifi(wifi_mode_type mode);