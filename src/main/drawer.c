#include "main/drawer.h"

#include "main/devices.h"
#include "main/main.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


static float draw_x;
static float draw_y;

static Window_Info *window;
static SDL_Renderer *renderer;


int drawer_init(Window_Info *w, SDL_Renderer *r) {
    window = w;
    renderer = r;

    draw_x = 0; 
    draw_y = 0; 


    return 0;
}


void drawer_rect(int x, int y, int width, int height, SDL_Color color) {
    SDL_SetRenderDrawColor(renderer, color.r, color.g, color.b, color.a);

    // Simple rectangle drawing.
    SDL_Rect rect = {
        x, y, width, height
    };
    SDL_RenderFillRect(renderer, &rect);
}

void drawer_graph_data(int x, int y, int width, int height, int length, Data_Point *data, Graph_Flags flags) {
    float pixel_width  = width;
    float pixel_height = height;

    // y = y + height;

    // These values dictate how grid looks, and they should depend on what is being graphed.
    float graph_min_y = -10.0;
    float graph_max_y = 30.0;
    float graph_cell_height = 5.0;


    int cell_count_y = (int)((graph_max_y - graph_min_y) / graph_cell_height);
    int pixel_cell_height = pixel_height / cell_count_y;

    float data2pix = pixel_cell_height / graph_cell_height;
    

    SDL_SetRenderDrawColor(renderer, 55, 55, 55, 255);

    // Drawing horizontal grid lines.
    SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255);
    for (int i = 0; i < cell_count_y + 1; i++) {
        SDL_RenderDrawLine(renderer, x, y + pixel_height - (i * pixel_cell_height), x + pixel_width, y + pixel_height - (i * pixel_cell_height));
    }

    // Drawing vertical grid lines, always 9.
    SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255);
    for (int i = 0; i < 9; i++) {
        SDL_RenderDrawLine(renderer, x + (pixel_width / 8) * i, y, x + (pixel_width / 8) * i, y + pixel_height);
    }

    // Drawing graphing line itsef.
    SDL_SetRenderDrawColor(renderer, 255, 55, 55, 255);


    // @Temporary: For now just draw temperature.
    float x_step = pixel_width / (DATA_LENGTH - 1);
    for (int i = 0; i < length - 1; i++) {
        SDL_RenderDrawLine(renderer, x + i * x_step, y + pixel_height - (data[i].temperature * data2pix), x + (i + 1) * x_step, y + pixel_height - (data[i + 1].temperature * data2pix));
    }


}


void drawer_text(int x, int y, char *text, TTF_Font *font, SDL_Color color) {
    // As TTF_RenderText_Solid could only be used on
    // SDL_Surface then it is neccessary to create the surface first.
    SDL_Surface* surface_text = TTF_RenderText_Solid(font, text, color); 

    // Convert it into a texture
    SDL_Texture* texture_text = SDL_CreateTextureFromSurface(renderer, surface_text);

    int w, h;

    TTF_SizeText(font, text, &w, &h);

    SDL_Rect rect = {
        x, y, w, h,
    };

    SDL_RenderCopy(renderer, texture_text, NULL, &rect);

    // Freeing surface and texture
    SDL_FreeSurface(surface_text);
    SDL_DestroyTexture(texture_text);
}

void drawer_text_centered(int x, int y, int width, char *text, TTF_Font *font, SDL_Color color) {
    // As TTF_RenderText_Solid could only be used on
    // SDL_Surface then it is neccessary to create the surface first.
    SDL_Surface* surface_text = TTF_RenderText_Solid(font, text, color); 

    // Convert it into a texture
    SDL_Texture* texture_text = SDL_CreateTextureFromSurface(renderer, surface_text);

    int w, h;

    TTF_SizeText(font, text, &w, &h);

    x = x + (width * 0.5f) - (w * 0.5f);

    SDL_Rect rect = {
        x, y, w, h,
    };

    SDL_RenderCopy(renderer, texture_text, NULL, &rect);

    // Freeing surface and texture
    SDL_FreeSurface(surface_text);
    SDL_DestroyTexture(texture_text);
}


void drawer_text_right(int x, int y, int width, char *text, TTF_Font *font, SDL_Color color) {
    // As TTF_RenderText_Solid could only be used on
    // SDL_Surface then it is neccessary to create the surface first.
    SDL_Surface* surface_text = TTF_RenderText_Solid(font, text, color); 

    // Convert it into a texture
    SDL_Texture* texture_text = SDL_CreateTextureFromSurface(renderer, surface_text);

    int w, h;

    TTF_SizeText(font, text, &w, &h);

    x = x + width - w;

    SDL_Rect rect = {
        x, y, w, h,
    };

    SDL_RenderCopy(renderer, texture_text, NULL, &rect);

    // Freeing surface and texture
    SDL_FreeSurface(surface_text);
    SDL_DestroyTexture(texture_text);
}


