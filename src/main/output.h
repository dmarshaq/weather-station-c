#ifndef WEATHER_STATION_OUTPUT_H
#define WEATHER_STATION_OUTPUT_H

#include "main/devices.h"

#include <stdio.h>

/**
 * Appends data point to csv file.
 * Properly formats and writes data.
 * If pointer to the csv file is NULL, early returns 0.
 * If error occured returns -1.
 * @Important: csv file is guranteed to be open in append mode.
 */
int output_append_data_point(FILE *csv, Data_Point *data_point);

#endif
