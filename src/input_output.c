/* IO setting from TTGO demo */

#include <driver/gpio.h>
#include <esp_system.h>

#include "input_output.h"
#include "fonts.h"
#include "graphics.h"

#include <freertos/FreeRTOS.h>
#include <freertos/event_groups.h>
#include <freertos/timers.h>
#include <freertos/task.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <math.h>
#include <esp_log.h>
#include <esp_sntp.h>
#include <nvs_flash.h>

// for button inputs
QueueHandle_t inputQueue;
TimerHandle_t repeatTimer;
uint64_t lastkeytime=0;
int keyrepeat=1;
static int button_val[2]={1,1};

static void repeatTimerCallback(TimerHandle_t pxTimer) {
    int v;
    if(button_val[0]==0) {
        v=0;
        xQueueSendFromISR(inputQueue,&v,0);
    }
    if(button_val[1]==0) {
        v=35;
        xQueueSendFromISR(inputQueue,&v,0);
    }
    xTimerChangePeriod( repeatTimer, pdMS_TO_TICKS(200), 0);
    xTimerStart( repeatTimer, 0 );

}

// interrupt handler for button presses on GPIO0(L) and GPIO35(R)
static void IRAM_ATTR gpio_isr_handler(void* arg)
{
    uint32_t gpio_num = (uint32_t) arg;
    int gpio_index=(gpio_num==35);
    int val=(1-button_val[gpio_index]);

    uint64_t time=esp_timer_get_time();
    uint64_t timesince=time-lastkeytime;

    if(timesince>500) { //= 0.5s
        int v=gpio_num+val*100;
        xQueueSendFromISR(inputQueue,&v,0);
        if(val==0 && keyrepeat) {
            xTimerChangePeriod( repeatTimer, pdMS_TO_TICKS(400), 0);
            xTimerStart( repeatTimer, 0 );
        }
        if(val==1 && keyrepeat) {
            xTimerStop( repeatTimer, 0 );
        }
        lastkeytime=time;
    }
    button_val[gpio_index]=val;
    
    gpio_set_intr_type(gpio_num,val==0?GPIO_INTR_HIGH_LEVEL:GPIO_INTR_LOW_LEVEL);

}

key_type get_input() {
    int key;
    if(xQueueReceive(inputQueue,&key,0)==pdFALSE)
        return NO_KEY;
    switch(key) {
        case 0: return LEFT_DOWN;
        case 35: return RIGHT_DOWN;
    }
    return NO_KEY;
}

void input_output_init() {
     // queue for button presses
    inputQueue = xQueueCreate(4,4);
    repeatTimer = xTimerCreate("repeat", pdMS_TO_TICKS(300),pdFALSE,(void*)0, repeatTimerCallback);
    // interrupts for button presses
    gpio_set_direction(0, GPIO_MODE_INPUT);
    gpio_set_direction(35, GPIO_MODE_INPUT);
    gpio_set_direction(27, GPIO_MODE_OUTPUT); // set to output
    gpio_set_intr_type(0, GPIO_INTR_LOW_LEVEL);
    gpio_set_intr_type(35, GPIO_INTR_LOW_LEVEL);
    gpio_set_level(27,0);  // init to 0
    gpio_install_isr_service(0);
    gpio_isr_handler_add(0, gpio_isr_handler, (void*) 0);
    gpio_isr_handler_add(35, gpio_isr_handler, (void*) 35);
}


// for fresh storage
nvs_handle_t storage_open(nvs_open_mode_t mode) {
    esp_err_t err;
    nvs_handle_t my_handle;
    err = nvs_open("storage", mode, &my_handle);
    if(err!=0) {
        nvs_flash_init();
        err = nvs_open("storage", mode, &my_handle);
        printf("err1: %d\n",err);
    }
    return my_handle;
}

int storage_read_int(char *name, int def) {
    nvs_handle_t handle=storage_open(NVS_READONLY);
    int32_t val=def;
    nvs_get_i32(handle, name, &val);
    nvs_close(handle);
    return val;
}

void storage_write_int(char *name, int val) {
    nvs_handle_t handle=storage_open(NVS_READWRITE);
    nvs_set_i32(handle, name, val);
    nvs_commit(handle);
    nvs_close(handle);
}

void storage_read_string(char *name, char *def, char *dest, int len) {
    nvs_handle_t handle=storage_open(NVS_READONLY);
    strncpy(dest,def,len);
    size_t length=len;
    nvs_get_str(handle, name, dest, &length);
    nvs_close(handle);
    printf("Read %s = %s\n",name,dest);
}

void storage_write_string(char *name, char *val) {
    nvs_handle_t handle=storage_open(NVS_READWRITE);
    nvs_set_str(handle, name, val);
    nvs_commit(handle);
    nvs_close(handle);
     printf("Wrote %s = %s\n",name,val);
}

void edit_stored_string(char *name, char *prompt) {
    char val[64];
    storage_read_string(name,"",val,sizeof(val));
    get_string(prompt,val,sizeof(val));
    storage_write_string(name,val);
}