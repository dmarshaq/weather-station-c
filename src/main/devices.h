#ifndef WEATHER_STATION_DEVICES_H
#define WEATHER_STATION_DEVICES_H

#include <stdio.h>
#include <time.h>

// As we start working with hardware this struct will expand.
// Essentially it will contain information about the opened text and device files.
typedef struct devices_info {
    FILE *csv_output;
} Devices_Info;



// Depending on the actual sensor's output 
// this struct might change in the future.
typedef struct data_point {
    float temperature;        // °C
    float humidity;           // %RH
    float wind_speed;         // m/s
    float wind_direction;     // degrees [0–360)
    float pressure;           // hPa
    float precipitation;      // mm
    float uv_index;           // 0–15

    struct tm timestamp;	  // Time when data point is recorded.
                              // Can be formatted to YYYY-MM-DD HH:MM:SS
} Data_Point;


/**
 * Inits the devices "module", essentially saves pointer internally for devices_info, so the rest of the devices calls know where to output data.
 * Returns -1 if error occured.
 */
int devices_init(Devices_Info *devices_info);

/**
 * Goes through sensors and outputs data into a specified data_point.
 * Returns -1 if error occured.
 */
int devices_collect_data(Data_Point *data_point);




#endif
