/*
 * NESL
 * Copyright (C) 2022 David Jolly
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
 * associated documentation files (the "Software"), to deal in the Software without restriction,
 * including without limitation the rights to use, copy, modify, merge, publish, distribute,
 * sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all copies or
 * substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED,
 * INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
 * PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
 * AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION
 * WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*!
 * @file sdl.c
 * @brief Common service.
 */

#include <SDL2/SDL.h>
#include <bus.h>

/*!
 * @union nesl_color_t
 * @brief Contains pixel color channels.
 */
typedef union {

    struct {
        uint8_t blue;                   /*!< Blue channel */
        uint8_t green;                  /*!< Green channel */
        uint8_t red;                    /*!< Red channel */
    };

    uint32_t raw;                       /*!< Raw word */
} nesl_color_t;

/*!
 * @struct nesl_service_t
 * @brief Contains the service contexts.
 */
typedef struct {
    uint32_t tick;                      /*!< Tick since last redraw */
    uint8_t scale;                      /*!< Scaling */
    nesl_color_t pixel[240][256];       /*!< Pixel buffer */
    SDL_JoystickID joystick;            /*!< Joystick ID */

    struct {
        SDL_AudioDeviceID audio;        /*!< Audio handle */
        SDL_GameController *controller; /*!< Controller handle */
        SDL_Cursor *cursor;             /*!< Cursor handle */
        SDL_Renderer *renderer;         /*!< Renderer handle */
        SDL_Texture *texture;           /*!< Texture handle */
        SDL_Window *window;             /*!< Window handle */
    } handle;
} nesl_service_t;

static nesl_service_t g_service = {};   /*!< Service context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Close audio device.
 */
static void nesl_service_close_audio(void)
{

    if(g_service.handle.audio) {
        SDL_PauseAudioDevice(g_service.handle.audio, 1);
        SDL_CloseAudioDevice(g_service.handle.audio);
        g_service.handle.audio = 0;
    }
}

/**
 * @brief Clear display.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_service_clear(void)
{

    for(int y = 0; y < 240; ++y) {

        for(int x = 0; x < 256; ++x) {
            nesl_service_set_pixel(63, false, false, false, x, y);
        }
    }

    return nesl_service_redraw();
}

bool nesl_service_get_button(nesl_button_e button)
{
    bool result = false;
    const uint32_t KEY[BUTTON_MAX] = {
        SDL_SCANCODE_L, SDL_SCANCODE_K, SDL_SCANCODE_C, SDL_SCANCODE_SPACE,
        SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D,
        };

    if(g_service.handle.controller) {
        const SDL_GameControllerButton KEY[BUTTON_MAX] = {
            SDL_CONTROLLER_BUTTON_A, SDL_CONTROLLER_BUTTON_B, SDL_CONTROLLER_BUTTON_BACK, SDL_CONTROLLER_BUTTON_START,
            SDL_CONTROLLER_BUTTON_DPAD_UP, SDL_CONTROLLER_BUTTON_DPAD_DOWN, SDL_CONTROLLER_BUTTON_DPAD_LEFT, SDL_CONTROLLER_BUTTON_DPAD_RIGHT,
            };

        result = SDL_GameControllerGetButton(g_service.handle.controller, KEY[button]) ? true : false;
    }

    if(!result) {
        result = SDL_GetKeyboardState(NULL)[KEY[button]] ? true : false;
    }

    return result;
}

bool nesl_service_get_sensor(void)
{
    int x, y;

    SDL_GetMouseState(&x, &y);

    return g_service.pixel[y / g_service.scale][x / g_service.scale].raw != 0xFFFEFEFF;
}

bool nesl_service_get_trigger(void)
{
    int x, y;

    return SDL_GetMouseState(&x, &y) & SDL_BUTTON_LMASK;
}

nesl_error_e nesl_service_initialize(const char *title, int linear, int scale)
{
    nesl_error_e result = NESL_SUCCESS;
    const char *controller_map[] = {
        "03000000790000001100000010010000,Retro Controller,a:b1,b:b2,back:b8,dpdown:+a1,dpleft:-a0,dpright:+a0,dpup:-a1,leftshoulder:b6,lefttrigger:b7,rightshoulder:b4,righttrigger:b5,start:b9,x:b0,y:b3,platform:Linux",
        };

    g_service.scale = scale;

    if(g_service.scale < 1) {
        g_service.scale = 1;
    } else if(g_service.scale > 8) {
        g_service.scale = 8;
    }

    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_GAMECONTROLLER | SDL_INIT_VIDEO)) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(!(g_service.handle.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256 * g_service.scale, 240 * g_service.scale, 0))) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(!(g_service.handle.renderer = SDL_CreateRenderer(g_service.handle.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC))) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_RenderSetLogicalSize(g_service.handle.renderer, 256, 240)) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_SetRenderDrawColor(g_service.handle.renderer, 0, 0, 0, 0)) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1") == SDL_FALSE) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, linear ? "1" : "0") == SDL_FALSE) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(!(g_service.handle.texture = SDL_CreateTexture(g_service.handle.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240))) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(!(g_service.handle.cursor = SDL_CreateSystemCursor(SDL_SYSTEM_CURSOR_CROSSHAIR))) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    SDL_SetCursor(g_service.handle.cursor);

    if((result = nesl_service_reset()) == NESL_FAILURE) {
        goto exit;
    }

    for(size_t index = 0; index < sizeof(controller_map) / sizeof(*controller_map); ++index) {

        if(SDL_GameControllerAddMapping(controller_map[index]) == -1) {
            result = SET_ERROR("%s", SDL_GetError());
            goto exit;
        }
    }

exit:
    return result;
}

nesl_error_e nesl_service_poll(void)
{
    SDL_Event event;
    nesl_error_e result = NESL_SUCCESS;

    while(SDL_PollEvent(&event)) {

        switch(event.type) {
            case SDL_CONTROLLERDEVICEADDED:

                if(!g_service.handle.controller && SDL_IsGameController(event.cdevice.which)) {
                    SDL_Joystick *joystick = NULL;

                    if(!(g_service.handle.controller = SDL_GameControllerOpen(event.cdevice.which))) {
                        result = SET_ERROR("%s", SDL_GetError());
                        goto exit;
                    }

                    if(!(joystick = SDL_GameControllerGetJoystick(g_service.handle.controller))) {
                        result = SET_ERROR("%s", SDL_GetError());
                        goto exit;
                    }

                    if((g_service.joystick = SDL_JoystickInstanceID(joystick)) == -1) {
                        result = SET_ERROR("%s", SDL_GetError());
                        goto exit;
                    }
                }
                break;
            case SDL_CONTROLLERDEVICEREMOVED:

                if(g_service.handle.controller && (g_service.joystick == event.cdevice.which)) {
                    SDL_GameControllerClose(g_service.handle.controller);
                    g_service.handle.controller = NULL;
                }
                break;
            case SDL_KEYUP:

                if(!event.key.repeat) {

                    switch(event.key.keysym.scancode) {
                        case SDL_SCANCODE_R:

                            if((result = nesl_bus_interrupt(INTERRUPT_RESET)) == NESL_FAILURE) {
                                goto exit;
                            }
                            break;
                        default:
                            break;
                    }
                }
                break;
            case SDL_QUIT:
                result = NESL_QUIT;
                goto exit;
            default:
                break;
        }
    }

exit:
    return result;
}

nesl_error_e nesl_service_redraw(void)
{
    uint32_t elapsed;
    nesl_error_e result = NESL_SUCCESS;

    if(SDL_UpdateTexture(g_service.handle.texture, NULL, (uint32_t *)g_service.pixel, 256 * sizeof(uint32_t))) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_RenderClear(g_service.handle.renderer)) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_RenderCopy(g_service.handle.renderer, g_service.handle.texture, NULL, NULL)) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if((elapsed = (SDL_GetTicks() - g_service.tick)) < (1000 / (float)60)) {
        SDL_Delay((1000 / (float)60) - elapsed);
    }

    SDL_RenderPresent(g_service.handle.renderer);
    g_service.tick = SDL_GetTicks();

exit:
    return result;
}

nesl_error_e nesl_service_reset(void)
{
    nesl_error_e result;

    if((result = nesl_service_clear()) == NESL_FAILURE) {
        goto exit;
    }

    nesl_service_close_audio();
    g_service.tick = 0;

exit:
    return result;
}

nesl_error_e nesl_service_set_audio(nesl_service_get_audio callback, void *context)
{
    nesl_error_e result = NESL_SUCCESS;
    SDL_AudioSpec desired = {}, obtained = {};

    desired.callback = callback;
    desired.channels = 1;
    desired.format = AUDIO_F32SYS;
    desired.freq = 44100;
    desired.samples = 512;
    desired.userdata = context;
    nesl_service_close_audio();

    if((g_service.handle.audio = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0)) <= 0) {
        result = SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    SDL_PauseAudioDevice(g_service.handle.audio, 0);

exit:
    return result;
}

void nesl_service_set_pixel(uint8_t color, bool red, bool green, bool blue, uint8_t x, uint8_t y)
{
    const uint32_t PALETTE[] = {
        0xFF656565, 0xFF002D69, 0xFF131F7F, 0xFF3C137C, 0xFF690B62, 0xFF730A37, 0xFF710F07, 0xFF5A1A00,
        0xFF342800, 0xFF0B3400, 0xFF003C00, 0xFF003D10, 0xFF003840, 0xFF000000, 0xFF000000, 0xFF000000,
        0xFFAEAEAE, 0xFF0F63B3, 0xFF4051D0, 0xFF7841CC, 0xFFA736A9, 0xFFC03470, 0xFFBD3C30, 0xFF9F4A00,
        0xFF6D5C00, 0xFF366D00, 0xFF077704, 0xFF00793D, 0xFF00727D, 0xFF000000, 0xFF000000, 0xFF000000,
        0xFFFEFEFF, 0xFF5DBCFF, 0xFF8FA1FF, 0xFFC890FF, 0xFFF785FA, 0xFFFF83C0, 0xFFFF8B7F, 0xFFEF9A49,
        0xFFBDAC2C, 0xFF81A855, 0xFF55C753, 0xFF3CC98C, 0xFF3EC2CD, 0xFF4E4E4E, 0xFF000000, 0xFF000000,
        0xFFFEFEFF, 0xFFBCDFFF, 0xFFD1D8FF, 0xFFE8D1FF, 0xFFFBCDFD, 0xFFFFCCE5, 0xFFFFCFCA, 0xFFF8D5B4,
        0xFFE4DCA8, 0xFFCCE3A9, 0xFFB9E8B8, 0xFFAEE8D0, 0xFFAFE5EA, 0xFFB6B5B6, 0xFF000000, 0xFF000000,
        };

    g_service.pixel[y][x].raw = PALETTE[color];

    if(red) {
        g_service.pixel[y][x].red = 0xFF;
    }

    if(green) {
        g_service.pixel[y][x].green = 0xFF;
    }

    if(blue) {
        g_service.pixel[y][x].blue = 0xFF;
    }
}

void nesl_service_uninitialize(void)
{
    nesl_service_close_audio();

    if(g_service.handle.controller) {
        SDL_GameControllerClose(g_service.handle.controller);
    }

    if(g_service.handle.cursor) {
        SDL_FreeCursor(g_service.handle.cursor);
    }

    if(g_service.handle.texture) {
        SDL_DestroyTexture(g_service.handle.texture);
    }

    if(g_service.handle.renderer) {
        SDL_DestroyRenderer(g_service.handle.renderer);
    }

    if(g_service.handle.window) {
        SDL_DestroyWindow(g_service.handle.window);
    }

    SDL_Quit();
    memset(&g_service, 0, sizeof(g_service));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
