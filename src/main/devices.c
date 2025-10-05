#include "main/devices.h"


int devices_init(Devices_Info *devices_info) {
    FILE *file = fopen("data.csv", "a");

    if(file==NULL) {
        return -1;
    }

    devices_info->csv_output = file;
    return 0;
}

int devices_collect_data(Data_Point *data_point) {
    // @Incomplete.
    return 0;
}
