#include "common.h"
#include "piezo.h"
#include "pwr_button.h"
#include "temperature.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

//------------------------------------------------------------------------------------------------------------------------

typedef enum {
    system_state_idle,
    system_state_reboot,
    system_state_power_toggle,
} system_state_t;

typedef enum {
    cmd_idle,
    cmd_worker_reboot,
    cmd_complete_reboot,
    cmd_worker_power_toggle,
    cmd_complete_power_toggle,
    cmd_reset,
} cmd_t;

//------------------------------------------------------------------------------------------------------------------------

void handle_complete_reboot()
{
  // Wait for piezo indications to finish
  while (!piezo_empty_queue()) {
      usleep(50000);
  }

  // ssh worker "reboot"

  system("reboot");
  exit(0);
}

//------------------------------------------------------------------------------------------------------------------------

void handle_complete_power_toggle()
{
  // Wait for piezo indications to finish
  while (!piezo_empty_queue()) {
      usleep(50000);
  }

  // ssh worker "halt"

  system("halt");
  exit(0);
}

//------------------------------------------------------------------------------------------------------------------------

void handle_button_reboot(button_t* btn_reboot, system_state_t* system_state)
{
    button_event_t event_reboot;
    static uint64_t reboot_request_ts = 0;
    static uint64_t reboot_confirmed_ts = 0;
    static cmd_t reboot_cmd = cmd_idle;

    /*** Reboot button ***/
    button_poll_event(btn_reboot, &event_reboot);

    // Ignore event following execution of command
    if (reboot_cmd == cmd_reset) {
        reboot_cmd = cmd_idle;
        return;
    }

    if (event_reboot.new_event) {
        // Reboot request
        if (event_reboot.state == button_state_released &&
            event_reboot.prev_state_duration != 0) {
            reboot_request_ts = monotonic_ts();
            if (reboot_cmd == cmd_idle) {
                if (event_reboot.prev_state_duration <= 4000000) {
                    reboot_cmd = cmd_worker_reboot;
                    printf("piezo_indication_worker_reboot_requested\n");
                    piezo_add_to_queue(piezo_indication_worker_reboot_requested);
                } else {
                    reboot_cmd = cmd_complete_reboot;
                    printf("piezo_indication_complete_reboot_requested\n");
                    piezo_add_to_queue(piezo_indication_complete_reboot_requested);
                }
            }
        }

        // Reboot confirmation
        if (event_reboot.state == button_state_pressed &&
            event_reboot.prev_state_duration != 0) {
                reboot_confirmed_ts = monotonic_ts();
                if (reboot_request_ts != 0) {
                    if ((reboot_confirmed_ts - reboot_request_ts) < 5000000) {
                        if (reboot_cmd == cmd_worker_reboot) {
                            printf("piezo_indication_worker_reboot_confirmed\n");
                            piezo_add_to_queue(piezo_indication_worker_reboot_confirmed);
                            *system_state = system_state_reboot;
                            reboot_cmd = cmd_reset;
                        } else if (reboot_cmd == cmd_complete_reboot) {
                            piezo_add_to_queue(piezo_indication_complete_reboot_confirmed);
                            printf("piezo_indication_complete_reboot_confirmed\n");
                            handle_complete_reboot();
                        }
                    } else {
                        reboot_cmd = cmd_idle;
                    }
                }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------

void handle_button_power_toggle(button_t* btn_power_toggle, system_state_t* system_state)
{
    button_event_t event_power_toggle;
    static uint64_t power_toggle_request_ts = 0;
    static uint64_t power_toggle_confirmed_ts = 0;
    static cmd_t power_toggle_cmd = cmd_idle;

    /*** power_toggle button ***/
    button_poll_event(btn_power_toggle, &event_power_toggle);

    // Ignore event following execution of command
    if (power_toggle_cmd == cmd_reset) {
        power_toggle_cmd = cmd_idle;
        return;
    }

    if (event_power_toggle.new_event) {
        // Power toggle request
        if (event_power_toggle.state == button_state_released &&
            event_power_toggle.prev_state_duration != 0) {
            power_toggle_request_ts = monotonic_ts();
            if (power_toggle_cmd == cmd_idle) {
                if (event_power_toggle.prev_state_duration <= 4000000) {
                    power_toggle_cmd = cmd_worker_power_toggle;
                    printf("piezo_indication_worker_power_toggle_requested\n");
                    piezo_add_to_queue(piezo_indication_worker_power_toggle_requested);
                } else {
                    power_toggle_cmd = cmd_complete_power_toggle;
                    printf("piezo_indication_complete_power_toggle_requested\n");
                    piezo_add_to_queue(piezo_indication_complete_power_toggle_requested);
                }
            }
        }

        // power toggle confirmation
        if (event_power_toggle.state == button_state_pressed &&
            event_power_toggle.prev_state_duration != 0) {
                power_toggle_confirmed_ts = monotonic_ts();
                if (power_toggle_request_ts != 0) {
                    if ((power_toggle_confirmed_ts - power_toggle_request_ts) < 5000000) {
                        if (power_toggle_cmd == cmd_worker_power_toggle) {
                            printf("piezo_indication_worker_power_toggle_confirmed\n");
                            piezo_add_to_queue(piezo_indication_worker_power_toggle_confirmed);
                            *system_state = system_state_power_toggle;
                            power_toggle_cmd = cmd_reset;
                        } else if (power_toggle_cmd == cmd_complete_power_toggle) {
                            piezo_add_to_queue(piezo_indication_complete_power_toggle_confirmed);
                            printf("piezo_indication_complete_power_toggle_confirmed\n");
                            handle_complete_power_toggle();
                        }
                    } else {
                        power_toggle_cmd = cmd_idle;
                    }
                }
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------

static bool fan_top = false;
static bool fan_rear = false;

void handle_system_state_idle(button_t* btn_reboot, button_t* btn_power_toggle, system_state_t* system_state)
{
    // Button state
    handle_button_reboot(btn_reboot, system_state);
    handle_button_power_toggle(btn_power_toggle, system_state);

    // Fan control state
    if (get_temperature(RASP_PI_CORE_TEMP_PATH) <= 57) {
        if (fan_top || fan_rear) {
            system("/sbin/relay_ctrl fan_top off");
            system("/sbin/relay_ctrl fan_rear off");
            fan_top = false;
            fan_rear = false;
        }
    }

    else if (get_temperature(RASP_PI_CORE_TEMP_PATH) >= 62) {
        if (!fan_top) {
            system("/sbin/relay_ctrl fan_top on");
            fan_top = true;
        }
    }

    else if (get_temperature(RASP_PI_CORE_TEMP_PATH) >= 67) {
        if (!fan_rear) {
            system("/sbin/relay_ctrl fan_rear on");
            fan_rear = true;
        }
    }
}

//------------------------------------------------------------------------------------------------------------------------

int main(int argc, char* argv[]) {
    system_state_t system_state = system_state_idle;

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

    // Ensure silent piezo at startup
    piezo_add_to_queue(piezo_indication_idle);

    while(true) {
        switch(system_state) {
            case system_state_idle:
                handle_system_state_idle(btn_reboot, btn_power_toggle, &system_state);
                break;
            case system_state_reboot:
                sleep(1);
                system_state = system_state_idle;
                break;
            case system_state_power_toggle:
                sleep(1);
                system_state = system_state_idle;
                break;
        }

        usleep(50000);
    }

    button_destroy(btn_reboot);
    button_destroy(btn_power_toggle);

    if(pthread_join(piezo_thread, NULL)) {
        perror("pthread_join");
    }

    printf("exiting\n");
    return 0;
}

//------------------------------------------------------------------------------------------------------------------------

