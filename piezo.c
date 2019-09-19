

#include "piezo.h"
#include "gpio.h"
#include <unistd.h>

//---------------------------------------------------------------------------------------------------------------------

#define GPIO_PIN_PIEZO (18)
#define SHORT_PATTERN_DURATION (1000)
#define LONG_PATTERN_DURATION (50000)
#define VERY_LONG_PATTERN_DURATION (100000)

//---------------------------------------------------------------------------------------------------------------------

static gpio_t* gpio_piezo = NULL;

typedef enum {
    repeated_pattern_short,
    repeated_pattern_long,
    repeated_pattern_very_long,
} repeated_pattern_t;

//---------------------------------------------------------------------------------------------------------------------

static void pwm_write(gpio_t* gpio, int width)
{
    /* This is not formally implemented with a duty cycle and a well
       defined 'period', but it is enough to do what's needed */

    // Set high
    gpio_set(gpio, true);

    // Pulse width
    int i;
    for(i=0; i < width; i++) {
        usleep(10);
    }

    // Set low
    gpio_set(gpio, false);
}

//---------------------------------------------------------------------------------------------------------------------

static void play_repeated_pattern(gpio_t* gpio, repeated_pattern_t pattern, int count)
{
    int i;
    for(i=0; i < count; i++) {
        switch (pattern) {
            case repeated_pattern_short:
                pwm_write(gpio, SHORT_PATTERN_DURATION);
                break;
            case repeated_pattern_long:
            pwm_write(gpio, LONG_PATTERN_DURATION);
                break;
            case repeated_pattern_very_long:
                pwm_write(gpio, VERY_LONG_PATTERN_DURATION);
                break;
        }

        // Silent period
        usleep(100000);
    }
}

//---------------------------------------------------------------------------------------------------------------------

void play_piezo_indication(piezo_indication_t indication)
{
    if (!gpio_piezo) {
        gpio_piezo = gpio_create(GPIO_PIN_PIEZO, gpio_direction_output, gpio_active_low_inactive);
    }

    switch (indication) {
        // Reboot
        case piezo_indication_worker_reboot_requested:
            /* . . . */
            play_repeated_pattern(gpio_piezo, repeated_pattern_short, 3);
            break;
        case piezo_indication_worker_reboot_confirmed:
            /* . . . ________*/
            play_repeated_pattern(gpio_piezo, repeated_pattern_short, 3);
            play_repeated_pattern(gpio_piezo, repeated_pattern_very_long, 1);
            break;
        case piezo_indication_complete_reboot_requested:
            /* . . . . . */
            play_repeated_pattern(gpio_piezo, repeated_pattern_short, 5);
            break;
        case piezo_indication_complete_reboot_confirmed:
            /* . . . . . ________ */
            play_repeated_pattern(gpio_piezo, repeated_pattern_short, 5);
            play_repeated_pattern(gpio_piezo, repeated_pattern_very_long, 1);
            break;

        // Power off
        case piezo_indication_worker_power_toggle_requested:
            /* ___ ___ ___ */
            play_repeated_pattern(gpio_piezo, repeated_pattern_long, 3);
            break;
        case piezo_indication_worker_power_toggle_confirmed:
            /* ___ ___ ___ ________*/
            play_repeated_pattern(gpio_piezo, repeated_pattern_long, 3);
            play_repeated_pattern(gpio_piezo, repeated_pattern_very_long, 1);
            break;
        case piezo_indication_complete_power_toggle_requested:
            /* ___ ___ ___ ___ ___ */
            play_repeated_pattern(gpio_piezo, repeated_pattern_long, 5);
            break;
        case piezo_indication_complete_power_toggle_confirmed:
            /* ___ ___ ___ ___ ___ ________*/
            play_repeated_pattern(gpio_piezo, repeated_pattern_long, 5);
            play_repeated_pattern(gpio_piezo, repeated_pattern_very_long, 1);
            break;

        // Alarms
        case piezo_indication_warning:
            /* . . . . . . . . . . . . . . . . . . . . */
            play_repeated_pattern(gpio_piezo, repeated_pattern_short, 20);
            break;
        case piezo_indication_critical: {
            /* . . ________ . . ________ . . ________ . . ________ . . ________ . . ________ */
            int i;
            for(i=0; i < 6; i++) {
                play_repeated_pattern(gpio_piezo, repeated_pattern_short, 2);
                play_repeated_pattern(gpio_piezo, repeated_pattern_very_long, 1);
            }   
            break;
        }
    }
}

//---------------------------------------------------------------------------------------------------------------------





