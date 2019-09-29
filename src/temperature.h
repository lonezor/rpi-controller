#pragma once

#include <stdbool.h>

#define RASP_PI_CORE_TEMP_PATH "/sys/devices/virtual/thermal/thermal_zone0/temp"

//---------------------------------------------------------------------------------------------------------------------

double get_temperature(const char* path);

//---------------------------------------------------------------------------------------------------------------------

