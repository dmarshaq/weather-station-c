#include "main/output.h"
#include "main/devices.h"
#include "core/log.h"

#include <stdio.h>
#include <string.h>
#include <unistd.h>



int output_flush_file_completely(FILE *csv) {
    fflush(csv);
    fsync(fileno(csv));
    return 0;
}


int output_append_data_point(FILE *csv, Data_Point *data_point) {
    if(csv == NULL){
        return -1;
    }

    if(fprintf(csv, "%.1f,%.1f,%.1f,%s,%.1f,%.1f,%.1f,%i-%02i-%02i,%02i:%02i:%02i\n",
        data_point->temperature,
        data_point->humidity,
        data_point->wind_speed,
        data_point->wind_direction,
        data_point->pressure,
        data_point->precipitation,
        data_point->uv_index,
        data_point->timestamp.tm_year + 1900,
        data_point->timestamp.tm_mon + 1, // tm_mon is zero based so just add 1.
        data_point->timestamp.tm_mday,
        data_point->timestamp.tm_hour,
        data_point->timestamp.tm_min,
        data_point->timestamp.tm_sec
    ) < 1){
        return -1;
    }

    output_flush_file_completely(csv);

    LOG_INFO("Successfully wrote data to the disk.");

    return 0;
}
