#ifndef WEATHER_STATION_DEVICES_H
#define WEATHER_STATION_DEVICES_H

#include <stdio.h>
#include <time.h>

#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>
#include <limits.h>
#include <libgen.h>
#include <sys/stat.h>
#include <termios.h>
#include <fcntl.h>


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

    union {
        int serial_port;
    };
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
    float temperature;        // °c
    float humidity;           // %rh
    float wind_speed;         // m/s
    char wind_direction[16];  // string
    float pressure;           // hpa
    float precipitation;      // mm
    float uv_index;           // 0–15

    struct tm timestamp;	  // time when data point is recorded.
                              // can be formatted to yyyy-mm-dd hh:mm:ss
} Data_Point;


/**
 * Inits the devices "module", essentially saves pointer internally for devices_info, so the rest of the devices calls know where to output data.
 * Returns -1 if error occured.
 */
int devices_init(Devices_Info *devices_info);

/**
 * Tries to detect every device and updates their state, returns non zero value if critical error occured.
 */
int devices_detect();

/**
 * Goes through sensors and outputs data into a specified data_point.
 * Returns -1 if error occured.
 */
int devices_collect_data(Data_Point *data_point);

void read_serial(int serial_port, Data_Point* current_data_point);
//Run tcflush(serial_port, TCIFLUSH) before running and after opening serial port



#endif
