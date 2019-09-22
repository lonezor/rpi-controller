#pragma once

#include <stdint.h>
#include <stdbool.h>

//---------------------------------------------------------------------------------------------------------------------

typedef enum {
    gpio_direction_input,
    gpio_direction_output,
} gpio_direction_t;

typedef enum {
    gpio_active_low_enabled,
    gpio_active_low_disabled,
} gpio_active_low_t;


typedef uint8_t gpio_pin_t;

typedef struct _gpio_t gpio_t;

//---------------------------------------------------------------------------------------------------------------------

gpio_t* gpio_create(gpio_pin_t pin, gpio_direction_t direction, gpio_active_low_t active_low);

void gpio_destroy(gpio_t* gpio, bool unexport);

void gpio_set(gpio_t* gpio, bool value);

bool gpio_get(gpio_t* gpio);

//---------------------------------------------------------------------------------------------------------------------

