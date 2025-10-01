#include "main/main.h"

#include "main/devices.h"
#include "main/output.h"

#include "core/log.h"
#include "core/core.h"
#include <SDL2/SDL.h>



static const u32 UPDATE_STEP_TIME = 10;     // 10 Milliseconds per single update loop.
                                            // Equal to about: 1 Frame / 0.01 Seconds = 100 FPS.


static Application_State app_state;


int main(void) {
    FILE* fptr = fopen("file.csv", "a");
    Data_Point stuff;

    time_t rawtime;
    struct tm *timeInfo;
    time(&rawtime);
    timeInfo = localtime(&rawtime);

    stuff.temperature = 1;
    stuff.humidity = 2;
    stuff.wind_speed = 3;
    stuff.wind_direction = 4;
    stuff.pressure = 5;
    stuff.precipitation = 6;
    stuff.uv_index = 7;
    stuff.timestamp = *timeInfo;
    output_append_data_point(fptr, &stuff);

    // Set all values to zero in app_state.
    app_state = (Application_State) {0};

    // Initializing SDL Video.
    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        LOG_ERROR("SDL_Init Error: %s.", SDL_GetError());
        return 1;
    }
    
    LOG_INFO("Initted SDL video.");

    // Creating SDL Window.
    app_state.window = SDL_CreateWindow(
        "SDL2 Window",
        SDL_WINDOWPOS_UNDEFINED,
        SDL_WINDOWPOS_UNDEFINED,
        640, 480,
        SDL_WINDOW_SHOWN
    );

    if (!app_state.window) {
        LOG_ERROR("SDL_CreateWindow Error: %s.", SDL_GetError());
        SDL_Quit();
        return 1;
    }

    LOG_INFO("Created SDL_Window.");

    // Creating SDL renderer.
    app_state.renderer = SDL_CreateRenderer(app_state.window, -1, SDL_RENDERER_SOFTWARE);
    if (!app_state.renderer) {
        LOG_ERROR("SDL_CreateRenderer Error: %s.", SDL_GetError());
        SDL_DestroyWindow(app_state.window);
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


    // Wait until window is closed.
    app_state.quit = 0;
    while (!app_state.quit) {
        // Time logic.
        app_state.time_info.current_time = SDL_GetTicks();
        app_state.time_info.accumilated_time += app_state.time_info.current_time - app_state.time_info.last_update_time;
        app_state.time_info.last_update_time = app_state.time_info.current_time;

        // If in total we didn't wait for 10 ms to pass we skip updating, to have each frame be executed consistently 1 frame each 10 ms. 
        // Assuming UPDATE_STEP_TIME = 10 ms.
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




        // Collecting data from devices.
        if (devices_collect_data(&app_state.current_data_point) != 0) {
            LOG_ERROR("Couldn't collect data points from devices.");
            goto error_return;
        }



        static const float output_period = 60.0f; // 60 seconds.
        static float output_timer = 0.0f; // Start at 0.
        
        // Outputting data to the .csv file, if such exists, every 60 seconds.
        if (output_timer > output_period) {
            if (output_append_data_point(app_state.devices_info.csv_output, &app_state.current_data_point) != 0) {
                LOG_ERROR("Couldn't output data point changes to csv file.");
                goto error_return;
            }
            output_timer = 0.0f;
        }
        output_timer += app_state.time_info.delta_time;




        // Clear screen with black.
        SDL_SetRenderDrawColor(app_state.renderer, 0, 0, 0, 255);
        SDL_RenderClear(app_state.renderer);

        // Set color to draw.
        SDL_SetRenderDrawColor(app_state.renderer, 255, 255, 255, 255);

        // Simple rectangle drawing.
        SDL_Rect rect = {
            20, 20, 40, 40
        };
        SDL_RenderFillRect(app_state.renderer, &rect);

        // Display rendered shapes on the scree.
        SDL_RenderPresent(app_state.renderer);

    }

    // Clean up.
    SDL_DestroyRenderer(app_state.renderer);
    SDL_DestroyWindow(app_state.window);
    SDL_Quit();

    return 0;


error_return:
    // Error return.
    SDL_DestroyRenderer(app_state.renderer);
    SDL_DestroyWindow(app_state.window);
    SDL_Quit();

    return 1; 
}
