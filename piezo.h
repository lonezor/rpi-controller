typedef enum {
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
    piezo_indication_warning, /* Functionality degraded */
    piezo_indication_critical, /* Critical system failure */
} piezo_indication_t;

void play_piezo_indication(piezo_indication_t indication);