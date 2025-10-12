#ifndef WEATHER_STATION_DRAWER
#define WEATHER_STATION_DRAWER

#include "core/core.h"

#include "main/devices.h"
#include "main/main.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>



// @Incomplete: Implement struct.
typedef struct data_graph_specification {

} Data_Graph_Specification;



/**
 * Inits drawer.
 */
int drawer_init(Window_Info *window, SDL_Renderer *renderer);

/**
 * Draws rectangle to the screen.
 */
void drawer_rect(int x, int y, int width, int height, SDL_Color color);

/**
 * Draws graph to the screen.
 */
void drawer_graph_data(int x, int y, int width, int height, Data_Stream *stream, Data_Graph_Specification specs);

/**
 * Draws specified string to the screen.
 */
void drawer_text(int x, int y, char *text, TTF_Font *font, SDL_Color color);

/**
 * Draws specified string center to the screen.
 */
void drawer_text_centered(int x, int y, int width, char *text, TTF_Font *font, SDL_Color color);

/**
 * Draws specified string justified right to the screen.
 */
void drawer_text_right(int x, int y, int width, char *text, TTF_Font *font, SDL_Color color);

#endif
