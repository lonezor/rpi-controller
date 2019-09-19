#include <stdio.h>

#include "piezo.h"

int main(int argc, char* argv[]) {

    printf("play...\n");
    play_piezo_indication(piezo_indication_worker_reboot_requested);
    printf("done\n");

    return 0;
}