#include "main/main.h"

#include "main/devices.h"
#include "main/drawer.h"
#include "main/output.h"

#include "core/log.h"
#include "core/core.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>



    // 500 Milliseconds per single update loop.
                                             // Equal to about: 1 Frame / 0.5 Seconds = 2 FPS.
                                             // Because sensors do not usually update faster then 0.5 seconds.


// Variables for styling, color values taken from design.
static const SDL_Color COLOR_TEXT = {216, 217, 218, 255};
static const SDL_Color COLOR_OK = {101, 201, 116, 255};
static const SDL_Color COLOR_ERR = {213, 111, 111, 255};
static const SDL_Color COLOR_PANEL = {33, 33, 36, 255};



static Data_Graph_Specification specs[7] = {
    { -10.0f, 30.0f, 5.0f,          offsetof(struct data_point, temperature),  ((SDL_Color) {240, 181, 74, 255}) },
    { 0.0f, 100.0f, 10.0f,          offsetof(struct data_point, humidity),  ((SDL_Color) {0, 122, 189, 255}) },
    { 0.0f, 27.0f, 3.0f,            offsetof(struct data_point, wind_speed),  ((SDL_Color) {64, 179, 157, 255}) },
    { 0.0f, 360.0f, 30.0f,          offsetof(struct data_point, wind_direction), ((SDL_Color) {160, 199, 64, 255}) },
    { 948.75f, 1048.75f, 10.0f,    offsetof(struct data_point, pressure), ((SDL_Color) {219, 111, 134, 255}) },
    { 0.0f, 90.0f, 15.0f,           offsetof(struct data_point, precipitation), ((SDL_Color) {195, 75, 165, 255}) },
    { 0.0f, 15.0f, 3.0f,            offsetof(struct data_point, uv_index), ((SDL_Color) {139, 119, 194, 255}) },
};

static int current_spec = 0;




static Application_State app_state;


int main(void) {

    // Set all values to zero in app_state.
    app_state = (Application_State) {0};

    SDL_setenv("SDL_VIDEODRIVER", "KMSDRM", 1);

    // Initializing SDL Video.
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOG_ERROR("SDL_Init Error: %s.", SDL_GetError());
        return 1;
    }
    
    LOG_INFO("Initted SDL video.");

    // Initializing SDL ttf.
    if (TTF_Init() != 0) {
        LOG_ERROR("TTF_Init Error: %s.", SDL_GetError());
        return 1;
    }
    
    LOG_INFO("Initted SDL ttf.");


    SDL_SetHint(SDL_HINT_VIDEO_HIGHDPI_DISABLED, "1");


    // Creating SDL Window.
    app_state.window.ptr = SDL_CreateWindow(
        "SDL2 Window",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        800, 480,
        SDL_WINDOW_SHOWN | SDL_WINDOW_FULLSCREEN
    );

    SDL_ShowCursor(SDL_DISABLE);


    if (!app_state.window.ptr) {
        LOG_ERROR("SDL_CreateWindow Error: %s.", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    LOG_INFO("Created SDL_Window.");

    // Creating SDL renderer.
    app_state.renderer = SDL_CreateRenderer(app_state.window.ptr, -1, SDL_RENDERER_SOFTWARE);
    if (!app_state.renderer) {
        LOG_ERROR("SDL_CreateRenderer Error: %s.", SDL_GetError());
        SDL_DestroyWindow(app_state.window.ptr);
        SDL_Quit();
        return 1;
    }

    LOG_INFO("Created SDL_Renderer.");


    
    // Initializing modules if required by their interface.
    if (devices_init(&app_state.devices_info) != 0) {
        LOG_ERROR("Couldn't initialize devices module.");
        goto error_return;
    }

    LOG_INFO("Initted devices module.");





    // Get font resources.
    TTF_Font *font_open_sans_16 = TTF_OpenFont("res/font/OpenSans.ttf", 16);
    TTF_Font *font_open_sans_14 = TTF_OpenFont("res/font/OpenSans.ttf", 14);

    if (font_open_sans_16 == NULL || font_open_sans_14 == NULL) {
        LOG_ERROR("Couldn't load font 'res/font/OpenSans.ttf', Error: %s.", SDL_GetError());
        goto error_return;
    }

    LOG_INFO("Successfully loaded font.");


    if (drawer_init(&app_state.window, app_state.renderer, font_open_sans_14) != 0) {
        LOG_ERROR("Couldn't initialize drawer module.");
        goto error_return;
    }

    LOG_INFO("Initted drawer module.");


    // Set current_data_point to be the first data point in the array.
    app_state.stream.current_data_point = app_state.stream.data;
    app_state.stream.data_tail = app_state.stream.data;


    // Wait until window is closed.
    app_state.quit = 0;
    while (!app_state.quit) {
        // Time logic.
        app_state.time_info.current_time = SDL_GetTicks();
        app_state.time_info.accumilated_time += app_state.time_info.current_time - app_state.time_info.last_update_time;
        app_state.time_info.last_update_time = app_state.time_info.current_time;

        if (app_state.time_info.accumilated_time < UPDATE_STEP_TIME) {
            continue;
        }

        app_state.time_info.delta_time_milliseconds = app_state.time_info.accumilated_time;
        app_state.time_info.delta_time = (float)(app_state.time_info.delta_time_milliseconds) / 1000.0f;
        app_state.time_info.accumilated_time = app_state.time_info.accumilated_time % UPDATE_STEP_TIME;



        
        // Pooling SDL events, for example: inputs.
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_QUIT)
                app_state.quit = 1;

            // @Important: This if allows to exist the application by pressing Escape while we develop it, in the final version of the program, you will not be able to do it.
            if (event.type == SDL_KEYDOWN && event.key.keysym.sym == SDLK_ESCAPE)
                app_state.quit = 1;
        }

        // Updating window size, if it changed or anything happnes.
        SDL_GetWindowSize(app_state.window.ptr, &app_state.window.width, &app_state.window.height);



        if (devices_detect() != 0) {
            LOG_ERROR("Couldn't detect devices, critical error occured.");
            goto error_return;
        }




        // Collecting data from devices.
        if (devices_collect_data(app_state.stream.current_data_point) != 0) {
            LOG_ERROR("Couldn't collect data points from devices.");
            goto error_return;
        }





        static const float output_period = 60.0f; // 60 seconds.
        static float output_timer = 0.0f; // Start at 0.
        
        // Outputting data to the .csv file, if such exists, every 60 seconds.
        if (output_timer > output_period) {
            if (output_append_data_point(app_state.devices_info.csv_output, app_state.stream.current_data_point) != 0) {
                LOG_WARNING("Couldn't output data point changes to csv file.");
            }
            output_timer = 0.0f;
        }
        output_timer += app_state.time_info.delta_time;






        int line_height = TTF_FontHeight(font_open_sans_14);


        // Clear screen with black.
        SDL_SetRenderDrawColor(app_state.renderer, 22, 23, 25, 255);
        SDL_RenderClear(app_state.renderer);

        // Health check.
        drawer_rect(10, 10, 340, 175, COLOR_PANEL);
        drawer_text_centered(10, 15, 340, "Health Check", font_open_sans_16, COLOR_TEXT);
        for (int i = 0; i < app_state.devices_info.devices_length; i++) {
            drawer_text(20, 42 + line_height * i, app_state.devices_info.devices[i].name, font_open_sans_16, COLOR_TEXT);
            switch (app_state.devices_info.devices[i].state) {
                case OFFLINE:
                    drawer_text_right(20, 42 + line_height * i, 320, "Offline", font_open_sans_14, COLOR_ERR);
                    break;
                case ONLINE:
                    drawer_text_right(20, 42 + line_height * i, 320, "Online", font_open_sans_14, COLOR_OK);
                    break;
            }
        }


        // Information.
        drawer_rect(360, 10, 430, 175, COLOR_PANEL);
        drawer_text_centered(360, 15, 430, "Information", font_open_sans_16, COLOR_TEXT);

        char value[16];


        drawer_text(370, 42 + line_height * 0, "Temperature", font_open_sans_14, current_spec == 0 ? specs[current_spec].color : COLOR_TEXT);
        snprintf(value, 16, "%.1f*C", app_state.stream.current_data_point->temperature);
        drawer_text(690, 42 + line_height * 0, value, font_open_sans_14, current_spec == 0 ? specs[current_spec].color : COLOR_TEXT);


        drawer_text(370, 42 + line_height * 1, "Humidity", font_open_sans_14, current_spec == 1 ? specs[current_spec].color : COLOR_TEXT);
        snprintf(value, 16, "%.0f%%RH", app_state.stream.current_data_point->humidity);
        
        drawer_text(690, 42 + line_height * 1, value, font_open_sans_14, current_spec == 1 ? specs[current_spec].color : COLOR_TEXT);

        drawer_text(370, 42 + line_height * 2, "Wind Speed", font_open_sans_14, current_spec == 2 ? specs[current_spec].color : COLOR_TEXT);
        snprintf(value, 16, "%.1fm/s", app_state.stream.current_data_point->wind_speed);
        drawer_text(690, 42 + line_height * 2, value, font_open_sans_14, current_spec == 2 ? specs[current_spec].color : COLOR_TEXT);

        drawer_text(370, 42 + line_height * 3, "Wind Direction", font_open_sans_14, current_spec == 3 ? specs[current_spec].color : COLOR_TEXT);
        snprintf(value, 16, "%.0f*", app_state.stream.current_data_point->wind_direction);
        drawer_text(690, 42 + line_height * 3, value, font_open_sans_14, current_spec == 3 ? specs[current_spec].color : COLOR_TEXT);

        drawer_text(370, 42 + line_height * 4, "Atmospheric Pressure", font_open_sans_14, current_spec == 4 ? specs[current_spec].color : COLOR_TEXT);
        snprintf(value, 16, "%.1fhPa", app_state.stream.current_data_point->pressure);
        drawer_text(690, 42 + line_height * 4, value, font_open_sans_14, current_spec == 4 ? specs[current_spec].color : COLOR_TEXT);

        drawer_text(370, 42 + line_height * 5, "Precipitation", font_open_sans_14, current_spec == 5 ? specs[current_spec].color : COLOR_TEXT);
        snprintf(value, 16, "%.1fmm", app_state.stream.current_data_point->precipitation);
        drawer_text(690, 42 + line_height * 5, value, font_open_sans_14, current_spec == 5 ? specs[current_spec].color : COLOR_TEXT);

        drawer_text(370, 42 + line_height * 6, "UV Intensity", font_open_sans_14, current_spec == 6 ? specs[current_spec].color : COLOR_TEXT);
        snprintf(value, 16, "%.0f", app_state.stream.current_data_point->uv_index);
        drawer_text(690, 42 + line_height * 6, value, font_open_sans_14, current_spec == 6 ? specs[current_spec].color : COLOR_TEXT);

        
        // Graph.
        drawer_rect(10, 195, 780, 275, COLOR_PANEL);
        drawer_graph_data(90, 210, 632, 232, &app_state.stream, specs[current_spec]);

        
        static const float spec_period = 8.0f;
        static float spec_timer = 0.0f; // Start at 0.
        
        // Changing what graph is displaying.
        if (spec_timer > spec_period) {
            current_spec++;
            if (current_spec > 6) {
                current_spec = 0;
            }
            spec_timer = 0.0f;
        }
        spec_timer += app_state.time_info.delta_time;


        drawer_text_centered(0, 600, app_state.window.width, "Press ESCAPE to exit        Demo of fake data displayed", font_open_sans_16, (SDL_Color) {255, 255, 255, 255});



        // Display rendered shapes on the scree.
        SDL_RenderPresent(app_state.renderer);



        // Move to the next data point in the array, wrap array if reached the end.
        app_state.stream.current_data_point++;
        if (app_state.stream.current_data_point - app_state.stream.data >= DATA_LENGTH) {
            app_state.stream.current_data_point = app_state.stream.data;
        }

        if (app_state.stream.current_data_point == app_state.stream.data_tail) {
            app_state.stream.data_tail++;
            if (app_state.stream.data_tail - app_state.stream.data >= DATA_LENGTH) {
                app_state.stream.data_tail = app_state.stream.data;
            }
        }

    }

    // Clean up.
    SDL_DestroyRenderer(app_state.renderer);
    SDL_DestroyWindow(app_state.window.ptr);
    SDL_Quit();

    return 0;


error_return:
    // Error return.
    SDL_DestroyRenderer(app_state.renderer);
    SDL_DestroyWindow(app_state.window.ptr);
    SDL_Quit();

    return 1; 
}
