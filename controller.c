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
    system_cmd_idle,
    system_cmd_worker_reboot,
    system_cmd_worker_power_toggle,
    system_cmd_complete_reboot,
    system_cmd_complete_power_toggle, // only power off is possible
    system_cmd_alarm_warning, 
    system_cmd_alarm_critical, 
} system_cmd_t;




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

    system_cmd_t reboot_cmd = system_cmd_idle;
    button_event_t event_reboot;
    uint64_t reboot_request_ts = 0;
    uint64_t reboot_confirmed_ts = 0;

    while(true) {

        /*** Reboot button ***/
        button_poll_event(btn_reboot, &event_reboot);

        if (event_reboot.new_event) {
            // Reboot request
            if (event_reboot.state == button_state_released && 
                event_reboot.prev_state_duration != 0) {
                    reboot_request_ts = monotonic_ts();
                    if (event_reboot.prev_state_duration <= 2000000) {
                        reboot_cmd = system_cmd_worker_reboot;
                        piezo_add_to_queue(piezo_indication_worker_reboot_requested);
                    } else {
                        reboot_cmd = system_cmd_complete_reboot;
                        piezo_add_to_queue(piezo_indication_complete_reboot_requested);
                    }                 
                }
            
            // Reboot confirmation
            if (event_reboot.state == button_state_pressed && 
                event_reboot.prev_state_duration != 0) {
                    reboot_confirmed_ts = monotonic_ts();
                    if (reboot_request_ts != 0) {
                        if ((reboot_confirmed_ts - reboot_request_ts) < 5000000) {
                            if (reboot_cmd == system_cmd_worker_reboot) {
                                piezo_add_to_queue(piezo_indication_worker_reboot_confirmed);
                            } else if (reboot_cmd == system_cmd_complete_reboot) {
                                piezo_add_to_queue(piezo_indication_complete_reboot_confirmed);
                            }
                        }
                    }
                }
        }

        usleep(10000);
    }

    button_destroy(btn_reboot);
    button_destroy(btn_power_toggle);

    if(pthread_join(piezo_thread, NULL)) {
        perror("pthread_join");
    }

    printf("exiting\n");
    return 0;
}
