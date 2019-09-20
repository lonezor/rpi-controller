#include "common.h"
#include <time.h>
#include <stdint.h>

#define DNS_NANOSEC_TO_MICROSEC_DIVIDER (1000LLU)
#define DNS_SEC_TO_MICROSEC_MULTIPLIER (1000000LLU)

//------------------------------------------------------------------------------------------------------------------------

uint64_t
monotonic_ts()
{
    uint64_t ts;
    struct timespec now;

    if (clock_gettime(CLOCK_MONOTONIC, &now)) {
        return 0;
    }

    ts = (uint64_t)now.tv_sec * DNS_SEC_TO_MICROSEC_MULTIPLIER;
    ts += (uint64_t)now.tv_nsec / DNS_NANOSEC_TO_MICROSEC_DIVIDER;

    return ts;
}

//------------------------------------------------------------------------------------------------------------------------

