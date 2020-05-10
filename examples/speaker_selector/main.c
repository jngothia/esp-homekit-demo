/*
 * jngothia's attempt to edit maximkulkin/esp-homekit-demo to work for my ESP8266 in order to create a A/B 
 * stereo selector switch, with a button and an LED for each relay.  Based on Maxim Kulkin's Sonoff S20 code.
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

// jngothia - Button code expects low voltage to indicate button is pressed(?), so button pins need to be pulled up
#include "button.h"

// D2 The GPIO pin that is connected to the relay on the ESP8266.
const int relay0_gpio = 4;
// D4 The GPIO pin that is connected to the LED on the ESP8266.  
// Note this pin must be held high with pullup resistor to ensure boot of ESP8266.
const int led0_gpio = 2;
// RX The GPIO pin that is connected to the button on the ESP8266.
const int button0_gpio = 3;

// jngothia b speakers
// D1
const int relay1_gpio = 5;
// D7
const int led1_gpio = 13;

// NEW TOGGLE CODE
#include "toggle.h"
// D5 The GPIO pins that are connected to the external buttons on the speaker selector.
const int toggle0_gpio = 14;
void toggle0_callback(uint8_t gpio);
// D6 jngothia next line addition of second toggle switch
const int toggle1_gpio = 12;
void toggle1_callback(uint8_t gpio);
//

// changed switch_on_callback to A_switch_on_callback, and button to button0, maybe unnecessary but wanted to avoid conflicts?
void A_switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
void button0_callback(uint8_t gpio, button_event_t event);
// jngothia added these, maybe unecessarily duplicative?
//void B_switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context);
//void B_button_callback(uint8_t gpio, button_event_t event);

// jngothia's ugly code with a function for writing to each pin)
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
    //Flash the LEDs first before we start the reset
    for (int i=0; i<3; i++) {
        led0_write(true);
        led1_write(false);
        vTaskDelay(100 / portTICK_PERIOD_MS);
        led0_write(false);
        led1_write(true);
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
    led0_write(A_switch_on.value.bool_value);
    gpio_enable(relay0_gpio, GPIO_OUTPUT);
    relay0_write(A_switch_on.value.bool_value);  // jngothia changed switch_on to A_switch_on
    // jngothia new switch for B speakers
    gpio_enable(led1_gpio, GPIO_OUTPUT);
    led1_write(B_switch_on.value.bool_value);
    gpio_enable(relay1_gpio, GPIO_OUTPUT);
    relay1_write(B_switch_on.value.bool_value);
    // NEW TOGGLE CODE
    gpio_enable(toggle0_gpio, GPIO_INPUT);
    //jngothia added next line toggle b
    gpio_enable(toggle1_gpio, GPIO_INPUT);
    //
}

// jngothia changed switch_on to A_switch_on
void A_switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
    relay0_write(A_switch_on.value.bool_value);
    led0_write(A_switch_on.value.bool_value);
}

//jngothia added B_switch_on
//void B_switch_on_callback(homekit_characteristic_t *_ch, homekit_value_t on, void *context) {
//    relay1_write(B_switch_on.value.bool_value);
//    led1_write(B_switch_on.value.bool_value);
//}

// jngothia changed button_callback to button0_callback
void button0_callback(uint8_t gpio, button_event_t event) {
    switch (event) {
        case button_event_single_press:
            printf("Switching relay A due to button0 at GPIO %2d\n", gpio);
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

// jngothia added B_button_callback
// void B_button_callback(uint8_t gpio, button_event_t event) {
//    switch (event) {
//       case button_event_single_press:
//            printf("Toggling relay B due to button at GPIO %2d\n", gpio);
//            B_switch_on.value.bool_value = !B_switch_on.value.bool_value;
//            relay1_write(B_switch_on.value.bool_value);
//            led1_write(B_switch_on.value.bool_value);
//            homekit_characteristic_notify(&B_switch_on, B_switch_on.value);
//            break;
//        case button_event_long_press:
//            reset_configuration();
//            break;
//        default:
//            printf("Unknown button event: %d\n", event);
//    }
//}

// NEW TOGGLE CODE
void toggle0_callback(uint8_t gpio) {
            printf("Toggling relay A due to toggle0 at GPIO %2d\n", gpio);
            A_switch_on.value.bool_value = !A_switch_on.value.bool_value;
            relay0_write(A_switch_on.value.bool_value);
            led0_write(A_switch_on.value.bool_value);
            homekit_characteristic_notify(&A_switch_on, A_switch_on.value);
}

//jngothia added toggle B code
void toggle1_callback(uint8_t gpio) {
            printf("Toggling relay B due to toggle1 at GPIO %2d\n", gpio);
            B_switch_on.value.bool_value = !B_switch_on.value.bool_value;
            relay1_write(B_switch_on.value.bool_value);
            led1_write(B_switch_on.value.bool_value);
            homekit_characteristic_notify(&B_switch_on, B_switch_on.value);
}

//

void switch_identify_task(void *_args) {
    // We identify the speaker selector by flashing it's LEDs.
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

    led0_write(A_switch_on.value.bool_value);
    led1_write(B_switch_on.value.bool_value);

    vTaskDelete(NULL);
}

void switch_identify(homekit_value_t _value) {
    printf("Switch identify\n");
    xTaskCreate(switch_identify_task, "Switch identify", 128, NULL, 2, NULL);
}

homekit_characteristic_t name = HOMEKIT_CHARACTERISTIC_(NAME, "Speaker Selector");

//homekit appears to arrange the services in the reverse order that they appear, hence A Speakers are defined last?
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

    if (button_create(button0_gpio, 0, 10000, button0_callback)) {
        printf("Failed to initialize button A\n");
    }
    
    if (toggle_create(toggle0_gpio, toggle0_callback)) {
        printf("Failed to initialize toggle A\n");
    }
    
    if (toggle_create(toggle1_gpio, toggle1_callback)) {
        printf("Failed to initialize toggle B\n");
    }
}
