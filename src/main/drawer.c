#include "main/drawer.h"

#include "main/devices.h"
#include "main/main.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>


static float draw_x;
static float draw_y;

static Window_Info *window;
static SDL_Renderer *renderer;
static TTF_Font *font;


int drawer_init(Window_Info *w, SDL_Renderer *r, TTF_Font *f) {
    window = w;
    renderer = r;

    draw_x = 0; 
    draw_y = 0; 

    font = f;


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

#define VERTICAL_LINE_TIME_INTERVAL 7.5f

void drawer_graph_data(int x, int y, int width, int height, Data_Stream *stream, Data_Graph_Specification specs) {
    // Buffer used to store formatted values.
    char buffer[128];

    // These values dictate how grid looks, and they should depend on what is being graphed.
    float graph_min_y = specs.min_y;
    float graph_max_y = specs.max_y;
    float graph_cell_height = specs.cell_height;
    u8 struct_member_value_offset = specs.struct_member_value_offset; // Offset in bytes to memory withing data_point struct where temperature float is located.


    int cell_count_y = (int)((graph_max_y - graph_min_y) / graph_cell_height);
    int pixel_cell_height = height / cell_count_y;

    float data2pix = pixel_cell_height / graph_cell_height;
    

    SDL_SetRenderDrawColor(renderer, 55, 55, 55, 255);

    // Drawing horizontal grid lines.
    SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255);
    for (int i = 0; i < cell_count_y + 1; i++) {
        SDL_RenderDrawLine(renderer, x, y + height - (i * pixel_cell_height), x + width, y + height - (i * pixel_cell_height));


        if (i % 2 == 0) {
            switch(struct_member_value_offset) {
                case offsetof(struct data_point, temperature):
                    snprintf(buffer, 128, "%.1f*C", (graph_cell_height * i) + graph_min_y);
                    break;
                case offsetof(struct data_point, humidity):
                    snprintf(buffer, 128, "%.0f%%RH", (graph_cell_height * i) + graph_min_y);
                    break;
                case offsetof(struct data_point, wind_speed):
                    snprintf(buffer, 128, "%.1fm/s", (graph_cell_height * i) + graph_min_y);
                    break;
                case offsetof(struct data_point, wind_direction):
                    snprintf(buffer, 128, "%.0f*", (graph_cell_height * i) + graph_min_y);
                    break;
                case offsetof(struct data_point, pressure):
                    snprintf(buffer, 128, "%.1fhPa", (graph_cell_height * i) + graph_min_y);
                    break;
                case offsetof(struct data_point, precipitation):
                    snprintf(buffer, 128, "%.1fmm", (graph_cell_height * i) + graph_min_y);
                    break;
                case offsetof(struct data_point, uv_index):
                    snprintf(buffer, 128, "%.0f", (graph_cell_height * i) + graph_min_y);
                    break;
            }


            drawer_text_right(x - 105, y + height - (i * pixel_cell_height) - 8, 100, buffer, font, (SDL_Color){105, 105, 105, 255});
        }
    }

    // Drawing vertical grid lines, always 9.
    SDL_SetRenderDrawColor(renderer, 105, 105, 105, 255);
    int vertical_lines = (float)GRAPH_TIME_LENGTH / (float)VERTICAL_LINE_TIME_INTERVAL;
    for (int i = 0; i < vertical_lines + 1; i++) {
        SDL_RenderDrawLine(renderer, x + (width / vertical_lines) * i, y, x + (width / vertical_lines) * i, y + height);
        

        if (i % 2 == 0) {
            snprintf(buffer, 128, "%ds", -(GRAPH_TIME_LENGTH) + (15 * i / 2));
            drawer_text(x + (width / 8) * i, y + height, buffer, font, (SDL_Color){105, 105, 105, 255});
        }
    }

    // Drawing graphing line itsef.
    SDL_SetRenderDrawColor(renderer, specs.color.r, specs.color.g, specs.color.b, 255);


    float x_step = (float)width / (DATA_LENGTH - 1);
    
    Data_Point *current = stream->current_data_point;
    Data_Point *next = current;
    
    for (int i = -1;;) {
        if (next == stream->data_tail) {
            break;
        }

        next--;
        i++;
        if (next < stream->data) {
            next = stream->data + DATA_LENGTH - 1;
        }

        // if (struct_member_value_offset == offsetof(struct data_point, pressure)) {
        //     printf("%.2f\n", ((*(float *)((u8 *)(current) + struct_member_value_offset))) * data2pix);
        //     printf("%.2f\n", ((*(float *)((u8 *)(current) + struct_member_value_offset))) * data2pix);
        // }

        SDL_RenderDrawLine(renderer, x + width - i * x_step, y + height - (((*(float *)((u8 *)(current) + struct_member_value_offset)) - graph_min_y) * data2pix), x + width - (i + 1) * x_step, y + height - ((*(float *)((u8 *)(next) + struct_member_value_offset) - graph_min_y) * data2pix));

        current = next;
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


