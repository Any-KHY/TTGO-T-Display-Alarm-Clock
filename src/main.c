/* 159236_Assignment_3_Any_KWOK_22000531 */

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include "freertos/task.h"
#include <driver/gpio.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <esp_sntp.h>
#include <esp_timer.h>
#include <driver/timer.h>

#include "fonts.h"
#include <graphics.h>

#include <nvs_flash.h>   // for real time clock
#include <time.h>        // for internal clock
#include <mqtt_client.h> // for MQTT

#include "input_output.h" // for TTGO DEMO files
#include "networking.h"   // for TTGO DEMO files

int year, month, day, hour, min;
bool alarm_set = false;
struct tm *tm_info;
time_t time_now;

static void alarm_mqtt_callback(int event_id, void *event_data)
{
    esp_mqtt_event_handle_t event = event_data;
    if (event_id == MQTT_EVENT_CONNECTED)
    {
        esp_mqtt_client_handle_t client = event->client;
        esp_mqtt_client_subscribe(client, "/a159236/alarm", 0);
    }
    else if (event_id == MQTT_EVENT_DATA)
    {
        event->data[event->data_len] = 0;
        snprintf(network_event, sizeof(network_event), "Alarm : %s\n", event->data);
        int yyyy, mm, dd, hh, minute;
        // extract the first 5 data if it's in the correct format
        if (sscanf(event->data, "%4d-%2d-%2d %2d:%2d", &yyyy, &mm, &dd, &hh, &minute) == 5)
        {
            year = yyyy;
            month = mm;
            day = dd;
            hour = hh;
            min = minute;
            alarm_set = true;
        }
    }
}

bool is_real_time() {
    time(&time_now);  // get the current time in sec (since 1970/1/1)
    tm_info = localtime(&time_now);
    return tm_info->tm_year >= (2016 - 1900); // updated
}

void alarm_clock()
{
    graphics_init();
    set_orientation(0);   // landscape
    mqtt_connect(alarm_mqtt_callback); // connect to Massey Wifi and MQTT server

    // Real time update
    setenv("TZ", "NZST-12:00:00NZDT-13:00:00,M9.5.0,M4.1.0", 0); // set time zone to NZ time
    tzset();

    int sntp_status = 0;

    while (1)
    {
        cls(0);
        if (xEventGroupGetBits(network_event_group) & CONNECTED_BIT)
        {
            // set real time clock
            if (sntp_status == 0)
            {
                esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
                esp_sntp_setservername(0, "pool.ntp.org");
                esp_sntp_init();
                sntp_status = 1;
            }
            time(&time_now); // get the current time in sec (since 1970/1/1)
            tm_info = localtime(&time_now);

            int current_year = tm_info->tm_year + 1900;
            int current_month = tm_info->tm_mon + 1;
            int current_day = tm_info->tm_mday;
            int current_hour = tm_info->tm_hour;
            int current_min = tm_info->tm_min;

            while (!is_real_time())
            {
                setFont(FONT_UBUNTU16);
                setFontColour(255, 255, 255);
                gprintf("\n  Updatinging to real time...\n");
                setFontColour(255, 0, 0);
                setFont(FreeSansBold18pt7b);
                gprintf("\n\n - - : - - : - - \n\n");
                flip_frame();
                vTaskDelay(500 / portTICK_PERIOD_MS);

                cls(0);
                flip_frame();
                vTaskDelay(500 / portTICK_PERIOD_MS);
            }

            // print alarm
            setFont(FONT_UBUNTU16);
            setFontColour(255, 255, 255);
            gprintf("\n  Alarm : %04d-%02d-%02d %02d:%02d\n", year, month, day, hour, min);

            setFont(FreeSansBold18pt7b);
            setFontColour(0, 255, 0);
            if (tm_info->tm_hour <= 12)
            {
                gprintf("\n\n%2d:%02d:%02d am", tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec);
            }
            else
            {
                gprintf("\n\n%2d:%02d:%02d pm", tm_info->tm_hour - 12, tm_info->tm_min, tm_info->tm_sec);
            } 

            // Alarm triggered 
            if (year == current_year && month == current_month && day == current_day && hour == current_hour && min == current_min && alarm_set)
            {
                uint64_t last_time = esp_timer_get_time();
                int time_interval = 0;
                bool red_bg = true;
                gpio_set_level(27, 1); // Turn on the GPIO output

                while (get_input() == NO_KEY)
                {
                    uint64_t current_time = esp_timer_get_time();
                    time_interval = current_time - last_time;

                    if (time_interval >= 300000) // 300ms interval
                    {
                        if (red_bg)
                        {
                            cls(rgbToColour(0, 255, 0)); // Green
                        }
                        else
                        {
                            cls(rgbToColour(255, 0, 0)); // Red
                        }
                        flip_frame();
                        red_bg = !red_bg;
                        last_time = current_time;
                    }

                    if (get_input() != NO_KEY)
                    {
                        alarm_set = false;
                        gpio_set_level(27, 0); // Turn off the GPIO output
                        flip_frame();
                        break;
                    }
                }
            }
            flip_frame();
        }
    }
}

void app_main() {
    // initialise button handling
    input_output_init();
    
    // Initialize NVS
    esp_err_t err = nvs_flash_init();

    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS partition was truncated and needs to be erased
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK( err );

    alarm_clock();
}
