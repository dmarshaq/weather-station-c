#include "main/devices.h"

#include <time.h>
#include <stdlib.h>


int devices_init(Devices_Info *devices_info) {
    srand(time(NULL));
    FILE *file = fopen("data.csv", "a");

    if(file == NULL) {
        return -1;
    }

    devices_info->csv_output = file;
    return 0;
}

int devices_collect_data(Data_Point *data_point) {
    if (data_point == NULL) {
        return -1;
    }

    // Used to approximate values
    // https://weatherspark.com/y/20372/Average-Weather-in-Buffalo-New-York-United-States-Year-Round

    data_point->temperature = 17.8f + ((float)rand() / (float)RAND_MAX * (26.1f - 17.8f));
    data_point->humidity = 0.0f + ((float)rand() / (float)RAND_MAX * (31.0f - 0.0f));
    data_point->wind_speed = 2.68f + ((float)rand() / (float)RAND_MAX * (5.32f - 2.68f));
    data_point->wind_direction = 0.0f + ((float)rand() / (float)RAND_MAX * (360.0f - 0.0f));
    data_point->pressure = 979.35f + ((float)rand() / (float)RAND_MAX * (1048.75f - 979.35f));
    data_point->precipitation = 30.48f + ((float)rand() / (float)RAND_MAX * (73.66f - 30.48f));
    data_point->uv_index = 0.0f + ((float)rand() / (float)RAND_MAX * (15.0f - 0.0f));

    time_t t = time(NULL);
    struct tm *local_time = localtime(&t);
    if (local_time != NULL) {
        data_point->timestamp = *local_time;
    }

    return 0;
}
