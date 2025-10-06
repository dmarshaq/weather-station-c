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
    TTF_Font *font_open_sans = TTF_OpenFont("res/font/OpenSans.ttf", 16);

    if (font_open_sans == NULL) {
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




        // Clear screen with black.
        SDL_SetRenderDrawColor(app_state.renderer, 0, 0, 0, 255);
        SDL_RenderClear(app_state.renderer);



        drawer_graph_data(app_state.current_data_point - app_state.data, app_state.data, GRAPH_FLAG_TEMPERATURE);

        drawer_text("Press ESCAPE to exit        Demo of fake data displayed", font_open_sans, (SDL_Color) {255, 255, 255, 255});

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
