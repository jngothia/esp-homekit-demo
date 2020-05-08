/*
 * jngothia's attempt to edit maximkulkin/esp-homekit-demo to work for my ESP8266
 */

#include <stdio.h>
#include <espressif/esp_wifi.h>
#include <espressif/esp_sta.h>
#include <espressif/esp_common.h>
#include <esp/uart.h>
#include <esp8266.h>
#include <FreeRTOS.h>
#include <task.h>

#include <homekit/homekit.h>
#include <homekit/characteristics.h>
#include <wifi_config.h>

#include "button.h"

// D2 The GPIO pin that is connected to the relay on the Sonoff Basic.
const int relay0_gpio = 4;
// D4 The GPIO pin that is connected to the LED on the Sonoff Basic.
const int led0_gpio = 2;
// D5 The GPIO pin that is connected to the button on the Sonoff Basic.
const int button0_gpio = 14;

// jngothia b speakers
// D1
const int relay1_gpio = 5;
// D7
const int led1_gpio = 13;
// D6
const int button1_gpio = 12;

// changed switch_on_callback to A_switch_on_callback, and button to A_button, maybe unnecessary but wanted to avoid conflicts?
void A_switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void A_button_callback(uint8_t gpio, button_event_t event);
// jngothia added these, maybe unecessarily duplicative?
void B_switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void B_button_callback(uint8_t gpio, button_event_t event);

void relay0_write(bool on) {
    gpio_write(relay0_gpio, on ? 1 : 0);
}

void led0_write(bool on) {
    gpio_write(led0_gpio, on ? 0 : 1);
}

void relay1_write(bool on) {
    gpio_write(relay1_gpio, on ? 1 : 0);
}

void led1_write(bool on) {
    gpio_write(led1_gpio, on ? 0 : 1);
}

void reset_configuration_task() {
    //Flash the LED first before we start the reset
    for (int i=0; i<3; i++) {
        led0_write(true);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        led1_write(false);
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }

    printf("Resetting Wifi Config\n");

    wifi_config_reset();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("Resetting HomeKit Config\n");

    homekit_server_reset();

    vTaskDelay(1000 / portTICK_PERIOD_MS);

    printf("Restarting\n");

    sdk_system_restart();

    vTaskDelete(NULL);
}

void reset_configuration() {
    printf("Resetting Speaker Selector Config\n");
    xTaskCreate(reset_configuration_task, "Reset configuration", 256, NULL, 2, NULL);
}

homekit_characteristic_t A_switch_on = HOMEKIT_CHARACTERISTIC_(
    ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(A_switch_on_callback)
);
homekit_characteristic_t B_switch_on = HOMEKIT_CHARACTERISTIC_(
    ON, false, .callback=HOMEKIT_CHARACTERISTIC_CALLBACK(B_switch_on_callback)
);

void gpio_init() {
    gpio_enable(led0_gpio, GPIO_OUTPUT);
    led0_write(false);
    gpio_enable(relay0_gpio, GPIO_OUTPUT);
    relay0_write(A_switch_on.value.bool_value);  // jngothia changed switch_on to A_switch_on
    // jngothia new switch
    gpio_enable(led1_gpio, GPIO_OUTPUT);
    led1_write(false);
    gpio_enable(relay1_gpio, GPIO_OUTPUT);
    relay1_write(B_switch_on.value.bool_value);
}

// jngothia changed switch_on to A_switch_on
void A_switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay0_write(A_switch_on.value.bool_value);
    led0_write(A_switch_on.value.bool_value);
}

//jngothia added B_switch_on
void B_switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay1_write(B_switch_on.value.bool_value);
    led1_write(B_switch_on.value.bool_value);
}

// jngothia changed button_callback to A_button_callback
void A_button_callback(uint8_t gpio, button_event_t event) {
    switch (event) {
        case button_event_single_press:
            printf("Toggling relay A\n");
            A_switch_on.value.bool_value = !A_switch_on.value.bool_value;
            relay0_write(A_switch_on.value.bool_value);
            led0_write(A_switch_on.value.bool_value);
            homekit_characteristic_notify(&A_switch_on, A_switch_on.value);
            break;
        case button_event_long_press:
            reset_configuration();
            break;
        default:
            printf("Unknown button event: %d\n", event);
    }
}

// jngothia changed button_callback to A_button_callback
void B_button_callback(uint8_t gpio, button_event_t event) {
    switch (event) {
        case button_event_single_press:
            printf("Toggling relay B\n");
            B_switch_on.value.bool_value = !B_switch_on.value.bool_value;
            relay1_write(B_switch_on.value.bool_value);
            led1_write(B_switch_on.value.bool_value);
            homekit_characteristic_notify(&B_switch_on, B_switch_on.value);
            break;
        case button_event_long_press:
            reset_configuration();
            break;
        default:
            printf("Unknown button event: %d\n", event);
    }
}


void switch_identify_task(void *_args) {
    // We identify the Sonoff by Flashing it's LED.
    for (int i=0; i<3; i++) {
        for (int j=0; j<2; j++) {
            led0_write(true);
            led1_write(false);
            vTaskDelay(100 / portTICK_PERIOD_MS);
            led0_write(false);
            led1_write(true);
            vTaskDelay(100 / portTICK_PERIOD_MS);
        }

        vTaskDelay(250 / portTICK_PERIOD_MS);
    }

    led0_write(false);
    led1_write(false);

    vTaskDelete(NULL);
}

void switch_identify(homekit_value_t _value) {
    printf("Switch identify\n");
    xTaskCreate(switch_identify_task, "Switch identify", 128, NULL, 2, NULL);
}

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "Speaker Selector");

homekit_accessory_t *accessories[] = {
    HOMEKIT_ACCESSORY(.id=1, .category=homekit_accessory_category_outlet, .services=(homekit_service_t*[]){
        HOMEKIT_SERVICE(ACCESSORY_INFORMATION, .characteristics=(homekit_characteristic_t*[]){
            &name,
            HOMEKIT_CHARACTERISTIC(MANUFACTURER, "jngothia"),
            HOMEKIT_CHARACTERISTIC(SERIAL_NUMBER, "4095159335A2"),
            HOMEKIT_CHARACTERISTIC(MODEL, "Speaker Selector"),
            HOMEKIT_CHARACTERISTIC(FIRMWARE_REVISION, "0.0.1"),
            HOMEKIT_CHARACTERISTIC(IDENTIFY, switch_identify),
            NULL
        }),
        HOMEKIT_SERVICE(OUTLET, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "B Speakers"),
            &B_switch_on,
 	      HOMEKIT_CHARACTERISTIC(OUTLET_IN_USE, true),
            NULL
        }),
        HOMEKIT_SERVICE(OUTLET, .primary=true, .characteristics=(homekit_characteristic_t*[]){
            HOMEKIT_CHARACTERISTIC(NAME, "A Speakers"),
            &A_switch_on,
 	      HOMEKIT_CHARACTERISTIC(OUTLET_IN_USE, true),
            NULL
        }),
        NULL
    }),
    NULL
};

homekit_server_config_t config = {
    .accessories = accessories,
    .password = "111-11-111"
};

void on_wifi_ready() {
    homekit_server_init(&config);
}

void create_accessory_name() {
    uint8_t macaddr[6];
    sdk_wifi_get_macaddr(STATION_IF, macaddr);

    int name_len = snprintf(NULL, 0, "Speaker Selector-%02X%02X%02X",
                            macaddr[3], macaddr[4], macaddr[5]);
    char *name_value = malloc(name_len+1);
    snprintf(name_value, name_len+1, "Speaker Selector-%02X%02X%02X",
             macaddr[3], macaddr[4], macaddr[5]);

    name.value = HOMEKIT_STRING(name_value);
}

void user_init(void) {
    uart_set_baud(0, 115200);

    create_accessory_name();

    wifi_config_init("speaker-selector", NULL, on_wifi_ready);
    gpio_init();

    if (button_create(button0_gpio, 0, 10000, A_button_callback)) {
        printf("Failed to initialize button A\n");
    }
    
    if (button_create(button1_gpio, 0, 10000, B_button_callback)) {
        printf("Failed to initialize button B\n");
    }
}
