#include <stdio.h>
#include <stdbool.h>
#include <stdint.h>
#include <time.h>
#include "piezo.h"
#include "button.h"
#include "common.h"
#include <unistd.h>
#include <pthread.h>

typedef enum {
    system_state_idle,
    system_state_worker_rebooting,
    system_state_worker_power_toggling,
} system_state_t;

typedef enum {
    cmd_idle,
    cmd_worker_reboot,
    cmd_complete_reboot,
    cmd_reset,
} cmd_t;


void handle_system_state_idle(button_t* btn_reboot, button_t* btn_power_toggle, system_state_t* system_state)
{
    button_event_t event_reboot;
    static uint64_t reboot_request_ts = 0;
    static uint64_t reboot_confirmed_ts = 0;
    static cmd_t cmd = cmd_idle;

    /*** Reboot button ***/
    button_poll_event(btn_reboot, &event_reboot);

    // Ignore event following execution of command
    if (cmd == cmd_reset) {
        cmd = cmd_idle;
        return;
    }

    if (event_reboot.new_event) {
        // Reboot request
        if (event_reboot.state == button_state_released && 
            event_reboot.prev_state_duration != 0) {
            reboot_request_ts = monotonic_ts();
            if (cmd == cmd_idle) {
                if (event_reboot.prev_state_duration <= 2000000) {
                    cmd = cmd_worker_reboot;
                    printf("piezo_indication_worker_reboot_requested\n");
                    //piezo_add_to_queue(piezo_indication_worker_reboot_requested);
                } else {
                    cmd = cmd_complete_reboot;
                    printf("piezo_indication_complete_reboot_requested\n");
                    //piezo_add_to_queue(piezo_indication_complete_reboot_requested);
                }
            }
        }
            
        // Reboot confirmation
        if (event_reboot.state == button_state_pressed && 
            event_reboot.prev_state_duration != 0) {
                reboot_confirmed_ts = monotonic_ts();
                if (reboot_request_ts != 0) {
                    if ((reboot_confirmed_ts - reboot_request_ts) < 5000000) {
                        if (cmd == cmd_worker_reboot) {
                            printf("piezo_indication_worker_reboot_confirmed\n");
                            //piezo_add_to_queue(piezo_indication_worker_reboot_confirmed);
                            *system_state = system_state_worker_rebooting;
                            cmd = cmd_reset;
                        } else if (cmd == cmd_complete_reboot) {
                            //piezo_add_to_queue(piezo_indication_complete_reboot_confirmed);
                            printf("piezo_indication_complete_reboot_confirmed\n");
                        }
                    }
                }
        }
    }
}


int main(int argc, char* argv[]) {

    // Init buttons
    button_t* btn_reboot = button_create(button_type_reboot);
    button_t* btn_power_toggle  = button_create(button_type_power_toggle);
    if (!btn_reboot || !btn_power_toggle) {
        printf("Error: Unable to create buttons\n");
        return 1;
    }

    // Start service threads
    pthread_t piezo_thread;
    if (pthread_create(&piezo_thread, NULL, piezo_play_thread_main, NULL)) {
        printf("Error: Unable to start piezo play thread\n");
        return 1;
    }

    system_state_t system_state = system_state_idle;

    while(true) {
        switch(system_state) {
            case system_state_idle:
                handle_system_state_idle(btn_reboot, btn_power_toggle, &system_state);
                break;
            case system_state_worker_rebooting:
                sleep(2);
                system_state = system_state_idle;
                break;
            case system_state_worker_power_toggling:
                break;
        }

        usleep(100000);
    }

    button_destroy(btn_reboot);
    button_destroy(btn_power_toggle);

    if(pthread_join(piezo_thread, NULL)) {
        perror("pthread_join");
    }

    printf("exiting\n");
    return 0;
}
