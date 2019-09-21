

#include "common.h"
#include "button.h"
#include "gpio.h"
#include <assert.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

#define GPIO_PIN_BUTTON_REBOOT (5)
#define GPIO_PIN_BUTTON_POWER_TOGGLE (6)

//------------------------------------------------------------------------------------------------------------------------

struct _button_t
{
    button_type_t type;
    gpio_t* gpio;
    uint64_t pressed_ts;
    uint64_t released_ts;
};

//------------------------------------------------------------------------------------------------------------------------

button_t* button_create(button_type_t type)
{
    button_t* b = calloc(1, sizeof(button_t));
    if (!b) {
        return NULL;
    }

    switch (type) {
        case button_type_reboot:
            b->gpio = gpio_create(GPIO_PIN_BUTTON_REBOOT, gpio_direction_input, gpio_active_low_enabled);
            break;

        case button_type_power_toggle:
            b->gpio = gpio_create(GPIO_PIN_BUTTON_POWER_TOGGLE, gpio_direction_input, gpio_active_low_enabled);
            break;
    }

    return b;
}

//------------------------------------------------------------------------------------------------------------------------

void button_destroy(button_t* btn)
{
    if (!btn) {
        return;
    }

    gpio_destroy(btn->gpio);
    free(btn);
}

//------------------------------------------------------------------------------------------------------------------------

button_state_t button_state(button_t* btn)
{
    return (gpio_get(btn->gpio) == true) ? button_state_pressed : button_state_released;
}

//------------------------------------------------------------------------------------------------------------------------

void button_poll_event(button_t* btn, button_event_t* event)
{
    memset(event, 0, sizeof(button_event_t));

    uint64_t duration;
    button_state_t state = button_state(btn);
  
        if (state == button_state_pressed) {
            if (!btn->pressed_ts) {
                btn->pressed_ts = monotonic_ts();
                
                duration = 0;
                if (btn->released_ts) {
                    duration = btn->pressed_ts - btn->released_ts;
                }
                
                event->new_event = true;
                event->type = btn->type;
                event->state = state;
                event->timestamp = btn->pressed_ts;
                event->prev_state_duration = duration;
            }

        } else { // button released
            if (btn->pressed_ts) {
                btn->released_ts = monotonic_ts();
                duration = btn->released_ts - btn->pressed_ts;

                event->new_event = true;
                event->type = btn->type;
                event->state = state;
                event->timestamp = btn->released_ts;
                event->prev_state_duration = duration;

                btn->pressed_ts = 0;
            }
        }
}

//------------------------------------------------------------------------------------------------------------------------

