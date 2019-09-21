

#include "piezo.h"
#include "gpio.h"
#include "adt.h"
#include <unistd.h>
#include <pthread.h>
#include <stdio.h>

#define GPIO_PIN_PIEZO (18)
#define SHORT_PATTERN_DURATION (100)
#define LONG_PATTERN_DURATION (2000)
#define VERY_LONG_PATTERN_DURATION (4000)

//---------------------------------------------------------------------------------------------------------------------

typedef struct _play_entry
{
    struct _play_entry* next;
    piezo_indication_t indication;
} play_entry_t;

typedef enum {
    repeated_pattern_short,
    repeated_pattern_long,
    repeated_pattern_very_long,
} repeated_pattern_t;

//---------------------------------------------------------------------------------------------------------------------

static gpio_t* gpio_piezo = NULL;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

static play_entry_t* play_queue = NULL;

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
        usleep(1);
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

static void play_piezo_indication(piezo_indication_t indication)
{
    if (!gpio_piezo) {
        gpio_piezo = gpio_create(GPIO_PIN_PIEZO, gpio_direction_output, gpio_active_low_disabled);
    }

    switch (indication) {
        case piezo_indication_idle:
            gpio_set(gpio_piezo, false);
            break;
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
            /* . . . . . . */
            play_repeated_pattern(gpio_piezo, repeated_pattern_short, 6);
            break;
        case piezo_indication_complete_reboot_confirmed:
            /* . . . . . . ________ */
            play_repeated_pattern(gpio_piezo, repeated_pattern_short, 6);
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
            /* ___ ___ ___ ___ ___ ___ */
            play_repeated_pattern(gpio_piezo, repeated_pattern_long, 6);
            break;
        case piezo_indication_complete_power_toggle_confirmed:
            /* ___ ___ ___ ___ ___ ___ ________*/
            play_repeated_pattern(gpio_piezo, repeated_pattern_long, 6);
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

static play_entry_t* create_entry(piezo_indication_t indication)
{
    play_entry_t* e = calloc(1, sizeof(play_entry_t));
    if (!e) {
        return NULL;
    }

    e->indication = indication;

    return e;
}

//---------------------------------------------------------------------------------------------------------------------

static void destroy_entry(entry_t* entry) {
    play_entry_t* e = (play_entry_t*)entry;
    free(e);
}

//---------------------------------------------------------------------------------------------------------------------

void piezo_add_to_queue(piezo_indication_t indication)
{
    play_entry_t* e = create_entry(indication);
    if (!e) {
        return;
    }

    pthread_mutex_lock(&mutex);
    adt_queue_push_back((entry_t**)&play_queue, (entry_t*)e);
    pthread_mutex_unlock(&mutex);
}

//---------------------------------------------------------------------------------------------------------------------

bool piezo_empty_queue()
{
    pthread_mutex_lock(&mutex);
    bool empty_queue = (play_queue == NULL);
    pthread_mutex_unlock(&mutex);

    return empty_queue;
}

//---------------------------------------------------------------------------------------------------------------------

void* piezo_play_thread_main(void* data)
{
    while(true) {
        // Don't busy loop
        usleep(50000);

        // Peek queue
        pthread_mutex_lock(&mutex);
        play_entry_t* e = (play_entry_t*)adt_queue_peek_front((const entry_t*)play_queue);
        pthread_mutex_unlock(&mutex);

        // Play until completion if entry is found
        if (e) {
            play_piezo_indication(e->indication);

            // Remove entry from queue to mark that it is fully processed
            pthread_mutex_lock(&mutex);
            e = (play_entry_t*)adt_queue_pop_front((entry_t**)&play_queue);
            pthread_mutex_unlock(&mutex);

            destroy_entry((entry_t*)e);
        }
    }

    return NULL;
}

//---------------------------------------------------------------------------------------------------------------------

