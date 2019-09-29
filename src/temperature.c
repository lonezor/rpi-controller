#include "temperature.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//---------------------------------------------------------------------------------------------------------------------

double get_temperature(const char* path)
{
    FILE* f = fopen(path, "r");

    int value = 0;
    fscanf(f, "%d\n", &value);

    double temp = (double)value / (double)1000;

    printf("temperature %f\n", temp);

    fclose(f);

    return temp;
}

//---------------------------------------------------------------------------------------------------------------------

