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
    gpio_active_low_t active_low;
    FILE* value;
};

//---------------------------------------------------------------------------------------------------------------------

gpio_t* gpio_create(gpio_pin_t pin, gpio_direction_t direction, gpio_active_low_t active_low)
{
    gpio_t* gpio = calloc(1, sizeof(gpio_t));
    if (!gpio) {
        return NULL;
    }

    gpio->pin = pin;
    gpio->direction = direction;
    gpio->active_low = active_low;

    struct stat buf;
    char dir[512];
    char cmd[512];

    snprintf(dir, sizeof(dir), "/sys/class/gpio/gpio%d", (int)pin);

    // Make pin available in file system
    if (stat(dir, &buf) != 0) {

        snprintf(cmd, sizeof(cmd), "echo \"%d\" > /sys/class/gpio/export", (int)pin);
        system(cmd);
    }

    // Set direction
    snprintf(cmd, sizeof(cmd), "echo \"%s\" > %s/direction",
                    direction == gpio_direction_input ? "in" : "out", dir);
    system(cmd);

    // Set active low
    snprintf(cmd, sizeof(cmd), "echo \"%d\" > %s/active_low",
            active_low == gpio_active_low_enabled ? 1 : 0, dir);
    system(cmd);

    // Open value file
    char value_path[512];
    snprintf(value_path, sizeof(value_path), "%s/value", dir);
    gpio->value = fopen(value_path, direction == gpio_direction_input ? "r" : "w");
    if (!gpio->value) {
        free(gpio);
        return NULL;
    }

    return gpio;
}

//---------------------------------------------------------------------------------------------------------------------

void gpio_destroy(gpio_t* gpio, bool unexport)
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
    char buffer [80];
    rewind(gpio->value);
    fscanf(gpio->value, "%s\n", buffer);
    return (strstr(buffer, "1") != NULL);
}

//---------------------------------------------------------------------------------------------------------------------
