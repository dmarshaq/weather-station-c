#include "main/devices.h"


int devices_init(Devices_Info *devices_info) {
    srand(time(NULL));
    FILE *file = fopen("data.csv", "a");

    if(file==NULL) {
        return -1;
    }

    devices_info->csv_output = file;
    return 0;
}

int devices_collect_data(Data_Point *data_point) {
    if(data_point==NULL) {
        return -1
    }

    //Used to approximate values
    //https://weatherspark.com/y/20372/Average-Weather-in-Buffalo-New-York-United-States-Year-Round

    data_point->temperature = 64.0f + ((float)rand() / (float)RAND_MAX * (79.0f - 64.0f));
    data_point->humidity = 0.0f + ((float)rand() / (float)RAND_MAX * (31.0f - 0.0f));
    data_point->wind_speed = 6.0f + ((float)rand() / (float)RAND_MAX * (11.9f - 6.0f));
    data_point->wind_direction = 0.0f + ((float)rand() / (float)RAND_MAX * (360.0f - 0.0f));
    data_point->pressure = 979.35f + ((float)rand() / (float)RAND_MAX * (1048.75f - 979.35f));
    data_point->precipitation = 30.48f + ((float)rand() / (float)RAND_MAX * (73.66f - 30.48f));
    data_point->uv_index = 0.0f + ((float)rand() / (float)RAND_MAX * (15.0f - 0.0f));

    time_t t = time(NULL);
    struct tm *localTime = localtime(&t);
    if(localTime!=NULL) {
        data_point->timestamp = *lt;
    }
    return 0;
}
