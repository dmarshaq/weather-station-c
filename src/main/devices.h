#ifndef WEATHER_STATION_DEVICES_H
#define WEATHER_STATION_DEVICES_H

#include <stdio.h>
#include <time.h>



typedef enum device_state {
    OFFLINE,
    ONLINE,
} Device_State;

typedef enum device_type {
    DEVICE_UNKNOWN,
    DEVICE_USB_DRIVE,
    DEVICE_I2C_SENSOR,
    DEVICE_ARDUINO,
} Device_Type;

typedef struct device {
    char name[32];
    Device_Type type;
    Device_State state;

    char path[128];
    int fd;
} Device;






#define MAX_DEVICES 8

typedef struct devices_info {
    int devices_length;
    Device devices[MAX_DEVICES];


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


int devices_detect();

/**
 * Tries to connect to every detected device,
 * it modifies devices state stored in the devices info.
 * If critical error occurs it returns -1.
 */
int devices_connect();

/**
 * Goes through sensors and outputs data into a specified data_point.
 * Returns -1 if error occured.
 */
int devices_collect_data(Data_Point *data_point);




#endif
