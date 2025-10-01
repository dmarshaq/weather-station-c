#include "main/output.h"
#include "main/devices.h"

#include <stdio.h>
#include <string.h>



int output_append_data_point(FILE *csv, Data_Point *data_point) {
    if(csv == NULL){
        return -1;
    }
    /*fprintf(csv, "%.1f, %c, %.1f, %c, %.1f, %c, %.1f, %c, %.1f, %c, %.1f, %c, %.1f, %c, %i, %c, %i, %c, %02i, %c, %i, %c, %02i, %c, %02i, %c",
        data_point->temperature, ',',
        data_point->humidity, ',',
        data_point->wind_speed, ',',
        data_point->wind_direction, ',',
        data_point->pressure, ',',
        data_point->precipitation, ',',
        data_point->uv_index, ',',
        data_point->timestamp.tm_year + 1900, '-',
        data_point->timestamp.tm_mon, '-',
        data_point->timestamp.tm_mday, ',',
        data_point->timestamp.tm_hour, ':',
        data_point->timestamp.tm_min, ':',
        data_point->timestamp.tm_sec, '\n'
    );*/
    fprintf(csv, "%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%.1f,%i-%02i-%02i,%02i:%02i:%02i\n",
        data_point->temperature,
        data_point->humidity,
        data_point->wind_speed,
        data_point->wind_direction,
        data_point->pressure,
        data_point->precipitation,
        data_point->uv_index,
        data_point->timestamp.tm_year + 1900,
        data_point->timestamp.tm_mon,
        data_point->timestamp.tm_mday,
        data_point->timestamp.tm_hour,
        data_point->timestamp.tm_min,
        data_point->timestamp.tm_sec
    );
    /*fputs(",", csv);
    fprintf(csv, "%.1f", data_point->humidity);
    fputs(",", csv);
    fprintf(csv, "%.1f", data_point->wind_speed);
    fputs(",", csv);
    fprintf(csv, "%.1f", data_point->wind_direction);
    fputs(",", csv);
    fprintf(csv, "%.1f", data_point->pressure);
    fputs(",", csv);
    fprintf(csv, "%.1f", data_point->precipitation);
    fputs(",", csv);
    fprintf(csv, "%.1f", data_point->uv_index);
    fputs(",", csv);

    fprintf(csv, "%i", data_point->timestamp.tm_year + 1900);
    fputs("-", csv);
    fprintf(csv, "%i", data_point->timestamp.tm_mon);
    fputs("-", csv);
    fprintf(csv, "%02i", data_point->timestamp.tm_mday);
    fputs(",", csv);
    

    fprintf(csv, "%i", data_point->timestamp.tm_hour);
    fputs(":", csv);
    fprintf(csv, "%02i", data_point->timestamp.tm_min);
    fputs(":", csv);
    fprintf(csv, "%02i", data_point->timestamp.tm_sec);

    fputs("\n", csv);*/
    return 0;
}
