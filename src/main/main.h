#ifndef WETHER_STATION_MAIN_H
#define WETHER_STATION_MAIN_H

#include "core/core.h"
#include "main/devices.h"

#include <stdio.h>
#include <time.h>
#include <SDL2/SDL.h>



typedef struct time_info {
    u32 current_time;
    u32 delta_time_milliseconds;
    float delta_time;

    u32 last_update_time;
    u32 accumilated_time;
} Time_Info;

typedef struct window_info {
    SDL_Window *ptr;
    int width;
    int height;
} Window_Info;

#define UPDATE_STEP_TIME 2000 
#define GRAPH_TIME_LENGTH 60
#define DATA_LENGTH ((GRAPH_TIME_LENGTH * 1000) / UPDATE_STEP_TIME)

typedef struct data_stream {
    Data_Point data[DATA_LENGTH];
    Data_Point *current_data_point;  // Current data that is collected in current frame.
    Data_Point *data_tail;           // Last data point.
} Data_Stream;




typedef struct application_state {
    Time_Info time_info;            // Stores information about frame time.

    Window_Info window;             // Information about the window.

    SDL_Renderer *renderer;         // Pointer to the renderer that draws to the screen.

    Devices_Info devices_info;      // Devices handler internal state.
    
    Data_Stream stream;

                                    // This information updates, as frame passes.

    int quit;                       // If set to 1 will close the application.
                                    // If set to 0, application continues to run.
} Application_State;

#endif
