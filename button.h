#pragma once

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    button_type_none = 0x0,
    button_type_reboot = 0x1,
    button_type_power_toggle = 0x2,
} button_type_t;

typedef enum {
    button_state_pressed,
    button_state_released,
} button_state_t;

typedef struct
{
    bool new_event;
    button_type_t type;
    button_state_t state;
    uint64_t timestamp;
    uint64_t prev_state_duration;
} button_event_t;

typedef struct _button_t button_t;

button_t* button_create(button_type_t type);

void button_destroy(button_t* btn);

button_state_t button_state(button_t* btn);

void button_poll_event(button_t* btn, button_event_t* event);





