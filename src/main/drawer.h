#ifndef WEATHER_STATION_DRAWER
#define WEATHER_STATION_DRAWER

#include "core/core.h"

#include "main/devices.h"
#include "main/main.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


typedef enum graph_flags : u8 {
    GRAPH_FLAG_TEMPERATURE      = 0x01,
    GRAPH_FLAG_HUMIDITY         = 0x02,
    GRAPH_FLAG_WIND_SPEED       = 0x04,
    GRAPH_FLAG_WIND_DIRECTION   = 0x08,
    GRAPH_FLAG_PRESSURE         = 0x10,
    GRAPH_FLAG_PRECIPITATION    = 0x20,
    GRAPH_FLAG_UV_INDEX         = 0x40,
} Graph_Flags;


int drawer_init(Window_Info *window, SDL_Renderer *renderer);

/**
 * Draws graph to the screen.
 */
void drawer_graph_data(int length, Data_Point *data, Graph_Flags flags);

/**
 * Draws specified string to the screen.
 */
void drawer_text(char *text, TTF_Font *font, SDL_Color color);


#endif
