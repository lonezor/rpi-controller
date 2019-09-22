#include "gpio.h"
#include <stdbool.h>
#include <stdio.h>
#include <string.h>

#define GPIO_PIN_FAN_UPPER_FRONT (13)
#define GPIO_PIN_FAN_LOWER_FRONT (19)
#define GPIO_PIN_FAN_REAR (26)
#define GPIO_PIN_FAN_TOP (16)
#define GPIO_PIN_WORKER_01 (20)

//---------------------------------------------------------------------------------------------------------------------

static void print_help()
{
    printf("Options:\n");
    printf(" - fan_upper_front <on/off>\n");
    printf(" - fan_lower_front <on/off>\n");
    printf(" - fan_rear <on/off>\n");
    printf(" - fan_top <on/off>\n");
    printf(" - worker_01 <on/off>\n");
}

//---------------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    if (argc < 3) {
        print_help();
        return 0;
    }

    bool enabled = strstr(argv[2], "on") ? true : false;
    int pin = -1;

    if (strstr(argv[1], "fan_upper_front")) {
        pin = GPIO_PIN_FAN_UPPER_FRONT;
    } else if (strstr(argv[1], "fan_lower_front")) {
        pin = GPIO_PIN_FAN_LOWER_FRONT;
    } else if (strstr(argv[1], "relay_channel_fan_rear")) {
        pin = GPIO_PIN_FAN_REAR;
    } else if (strstr(argv[1], "relay_channel_fan_top")) {
        pin = GPIO_PIN_FAN_TOP;
    } else if (strstr(argv[1], "relay_channel_worker_01")) {
        pin = GPIO_PIN_FAN_TOP;
    }

    if (pin != -1) {
        gpio_t* gpio = gpio_create(GPIO_PIN_FAN_UPPER_FRONT, gpio_direction_output, gpio_active_low_enabled);
        if (gpio) {
            gpio_set(gpio, enabled);
            gpio_destroy(gpio);
        }
    } else {
        printf("Unknown command\n");
    }

    return 0;
}

//---------------------------------------------------------------------------------------------------------------------

