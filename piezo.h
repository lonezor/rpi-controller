typedef enum {
    piezo_indication_idle, // no sound

    // Rebooting
    piezo_indication_worker_reboot_requested, /* button released within 2s */
    piezo_indication_worker_reboot_confirmed, /* button pressed within 5s after request */
    piezo_indication_complete_reboot_requested, /* button released after 4s */
    piezo_indication_complete_reboot_confirmed, /* button pressed within 5s after request */

    // Power toggle (on -> off, off -> on)
    piezo_indication_worker_power_toggle_requested, /* button released within 2s */
    piezo_indication_worker_power_toggle_confirmed, /* button pressed within 5s after request */
    piezo_indication_complete_power_toggle_requested, /* button released after 4s */
    piezo_indication_complete_power_toggle_confirmed, /* button pressed within 5s after request */

    // Alarms
    piezo_indication_warning, /* Functionality degraded. Testable using both buttons released after 4s */
    piezo_indication_critical, /* Critical system failure. Testable using both button released after 8s */
} piezo_indication_t;

void piezo_add_to_queue(piezo_indication_t indication);

void* piezo_play_thread_main(void* data);