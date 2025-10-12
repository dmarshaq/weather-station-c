#include "main/main.h"

#include "main/devices.h"
#include "main/drawer.h"
#include "main/output.h"

#include "core/log.h"
#include "core/core.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_ttf.h>



static const u32 UPDATE_STEP_TIME = 500;     // 500 Milliseconds per single update loop.
                                             // Equal to about: 1 Frame / 0.5 Seconds = 2 FPS.
                                             // Because sensors do not usually update faster then 0.5 seconds.


static Application_State app_state;


int main(void) {

    // Set all values to zero in app_state.
    app_state = (Application_State) {0};

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

    // Creating SDL Window.
    app_state.window.ptr = SDL_CreateWindow(
        "SDL2 Window",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_FULLSCREEN
    );

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


    if (drawer_init(&app_state.window, app_state.renderer) != 0) {
        LOG_ERROR("Couldn't initialize drawer module.");
        goto error_return;
    }

    LOG_INFO("Initted drawer module.");



    // Get font resources.
    TTF_Font *font_open_sans_16 = TTF_OpenFont("res/font/OpenSans.ttf", 16);
    TTF_Font *font_open_sans_14 = TTF_OpenFont("res/font/OpenSans.ttf", 14);

    if (font_open_sans_16 == NULL || font_open_sans_14 == NULL) {
        LOG_ERROR("Couldn't load font 'res/font/OpenSans.ttf', Error: %s.", SDL_GetError());
        goto error_return;
    }

    LOG_INFO("Successfully loaded font.");



    // Set current_data_point to be the first data point in the array.
    app_state.current_data_point = app_state.data;


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




        // Detecting devices. In case any new will appear
        static const float detect_period = 5.0f; // 5 seconds.
        static float detect_timer = 15.1f; // Start at 15.1, to execute at first frame.

        // Outputting data to the .csv file, if such exists, every 60 seconds.
        if (detect_timer > detect_period) {
            if (devices_detect() != 0) {
                LOG_ERROR("Couldn't detect devices, critical error occured.");
                goto error_return;
            }
            detect_timer = 0.0f;
        }
        detect_timer += app_state.time_info.delta_time;




        // Collecting data from devices.
        if (devices_collect_data(app_state.current_data_point) != 0) {
            LOG_ERROR("Couldn't collect data points from devices.");
            goto error_return;
        }





        static const float output_period = 60.0f; // 60 seconds.
        static float output_timer = 0.0f; // Start at 0.
        
        // Outputting data to the .csv file, if such exists, every 60 seconds.
        if (output_timer > output_period) {
            if (output_append_data_point(app_state.devices_info.csv_output, app_state.current_data_point) != 0) {
                LOG_ERROR("Couldn't output data point changes to csv file.");
                goto error_return;
            }
            output_timer = 0.0f;
        }
        output_timer += app_state.time_info.delta_time;




        // Move to the next data point in the array, wrap array if reached the end.
        app_state.current_data_point++;
        if (app_state.current_data_point - app_state.data >= DATA_LENGTH) {
            app_state.current_data_point = app_state.data;
        }


        SDL_Rect rect;
        SDL_Color text_color = {216, 217, 218, 255};
        SDL_Color ok_color = {101, 201, 116, 255};
        SDL_Color err_color = {213, 111, 111, 255};
        SDL_Color panel_color = {33, 33, 36, 255};

        int line_height = TTF_FontHeight(font_open_sans_14);


        // Clear screen with black.
        SDL_SetRenderDrawColor(app_state.renderer, 22, 23, 25, 255);
        SDL_RenderClear(app_state.renderer);

        // Health check.
        drawer_rect(10, 10, 340, 175, panel_color);
        drawer_text_centered(10, 15, 340, "Health Check", font_open_sans_16, text_color);
        for (int i = 0; i < app_state.devices_info.devices_length; i++) {
            drawer_text(20, 42 + line_height * i, app_state.devices_info.devices[i].name, font_open_sans_16, text_color);
            switch (app_state.devices_info.devices[i].state) {
                case OFFLINE:
                    drawer_text_right(20, 42 + line_height * i, 320, "Offline", font_open_sans_14, err_color);
                    break;
                case ONLINE:
                    drawer_text_right(20, 42 + line_height * i, 320, "Online", font_open_sans_14, ok_color);
                    break;
            }
        }


        // Information.
        drawer_rect(360, 10, 430, 175, panel_color);
        drawer_text_centered(360, 15, 430, "Information", font_open_sans_16, text_color);

        char value[16];


        drawer_text(370, 42 + line_height * 0, "Temperature", font_open_sans_14, text_color);
        snprintf(value, 16, "%.1f*C", app_state.current_data_point->temperature);
        drawer_text(690, 42 + line_height * 0, value, font_open_sans_14, text_color);


        drawer_text(370, 42 + line_height * 1, "Humidity", font_open_sans_14, text_color);
        snprintf(value, 16, "%.0f\%RH", app_state.current_data_point->humidity);
        drawer_text(690, 42 + line_height * 1, value, font_open_sans_14, text_color);

        drawer_text(370, 42 + line_height * 2, "Wind Speed", font_open_sans_14, text_color);
        snprintf(value, 16, "%.1fm/s", app_state.current_data_point->wind_speed);
        drawer_text(690, 42 + line_height * 2, value, font_open_sans_14, text_color);

        drawer_text(370, 42 + line_height * 3, "Wind Direction", font_open_sans_14, text_color);
        snprintf(value, 16, "%.0f*", app_state.current_data_point->wind_direction);
        drawer_text(690, 42 + line_height * 3, value, font_open_sans_14, text_color);

        drawer_text(370, 42 + line_height * 4, "Atmospheric Pressure", font_open_sans_14, text_color);
        snprintf(value, 16, "%.1fhPa", app_state.current_data_point->pressure);
        drawer_text(690, 42 + line_height * 4, value, font_open_sans_14, text_color);

        drawer_text(370, 42 + line_height * 5, "Precipitation", font_open_sans_14, text_color);
        snprintf(value, 16, "%.1fmm", app_state.current_data_point->precipitation);
        drawer_text(690, 42 + line_height * 5, value, font_open_sans_14, text_color);

        drawer_text(370, 42 + line_height * 6, "UV Intensity", font_open_sans_14, text_color);
        snprintf(value, 16, "%.0f", app_state.current_data_point->uv_index);
        drawer_text(690, 42 + line_height * 6, value, font_open_sans_14, text_color);

        
        // Graph.
        drawer_rect(10, 195, 780, 275, panel_color);
        drawer_graph_data(80, 210, 632, 232, app_state.current_data_point - app_state.data, app_state.data, GRAPH_FLAG_TEMPERATURE);


        drawer_text_centered(0, 600, app_state.window.width, "Press ESCAPE to exit        Demo of fake data displayed", font_open_sans_16, (SDL_Color) {255, 255, 255, 255});



        // Display rendered shapes on the scree.
        SDL_RenderPresent(app_state.renderer);

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
