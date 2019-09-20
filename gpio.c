#include "gpio.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

/* Make use of the sys file system that exists on rpi.
   This may need to be updated in a later Raspberry Pi release */

//---------------------------------------------------------------------------------------------------------------------

struct _gpio_t {
    gpio_pin_t pin;
    gpio_direction_t direction;
    gpio_active_t active;
    FILE* value;
};

//---------------------------------------------------------------------------------------------------------------------

gpio_t* gpio_create(gpio_pin_t pin, gpio_direction_t direction, gpio_active_t active)
{
    gpio_t* gpio = calloc(1, sizeof(gpio_t));
    if (!gpio) {
        return NULL;
    }

    char dir[512];
    snprintf(dir, sizeof(dir), "/sys/class/gpio/gpio%d", (int)pin);

    struct stat buf;
    if (stat(dir, &buf) != 0) {
        char cmd[512];
        
        // Make pin available in file system
        snprintf(cmd, sizeof(cmd), "echo \"%d\" > /sys/class/gpio/export", (int)pin);
        system(cmd);

        // Set direction
        snprintf(cmd, sizeof(cmd), "echo \"%s\" > %s/direction",
                    direction == gpio_direction_input ? "in" : "out", dir);
        system(cmd);

        // Set active low
        snprintf(cmd, sizeof(cmd), "echo \"%d\" > %s/active_low",
            active == gpio_active_high ? 1 : 0, dir);
        system(cmd);
	}

    char value_path[512];
    snprintf(value_path, sizeof(value_path), "%s/value", dir);
    gpio->value = fopen(value_path, "w");
    if (!gpio->value) {
        free(gpio);
        return NULL;
    }

    return gpio;
}

//---------------------------------------------------------------------------------------------------------------------

void gpio_destroy(gpio_t* gpio)
{
    if (!gpio) {
        return;
    }

    fclose(gpio->value);
        
    // Remove pin from file system
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "echo \"%d\" > /sys/class/gpio/unexport", (int)gpio->pin);
    system(cmd);
    
    free(gpio);
}

//---------------------------------------------------------------------------------------------------------------------

void gpio_set(gpio_t* gpio, bool value)
{
    fprintf(gpio->value, value ? "1" : "0");
    fflush(gpio->value);
}

//---------------------------------------------------------------------------------------------------------------------

bool gpio_get(gpio_t* gpio)
{
    char buffer[2] = {0,0};
    fread(buffer, sizeof(buffer), 0, gpio->value);
    return (strstr(buffer, "1") != NULL);
}

//---------------------------------------------------------------------------------------------------------------------