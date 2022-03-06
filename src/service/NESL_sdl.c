/**
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

/**
 * @file NESL_sdl.c
 * @brief SDL implementation of the service interface.
 */

#include <SDL2/SDL.h>
#include "../../include/system/NESL_input.h"
#include "../../include/NESL_service.h"

/**
 * @union nesl_color_t
 * @brief Contains pixel color channels.
 */
typedef union {

    struct {
        uint8_t blue;   /*< Blue channel */
        uint8_t green;  /*< Green channel */
        uint8_t red;    /*< Red channel */
    };

    uint32_t raw;
} nesl_color_t;

/**
 * @struct nesl_service_t
 * @brief Contains the service contexts.
 */
typedef struct {
    uint32_t tick;                  /*< Tick since last redraw */
    bool fullscreen;                /*< Fullscreen state */
    nesl_color_t pixel[240][256];   /*< Pixel buffer */
    SDL_AudioDeviceID audio;        /*< Open audio device id */
    SDL_Renderer *renderer;         /*< Renderer handle */
    SDL_Texture *texture;           /*< Texture handle */
    SDL_Window *window;             /*< Window handle */
} nesl_service_t;

static nesl_service_t g_service = {};   /*< Service context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void NESL_ServiceCloseAudio(void)
{

    if(g_service.audio) {
        SDL_PauseAudioDevice(g_service.audio, 1);
        SDL_CloseAudioDevice(g_service.audio);
        g_service.audio = 0;
    }
}

static nesl_error_e NESL_ServiceClear(void)
{

    for(int y = 0; y < 240; ++y) {

        for(int x = 0; x < 256; ++x) {
            NESL_ServiceSetPixel(63, false, false, false, x, y);
        }
    }

    return NESL_ServiceRedraw();
}

static nesl_error_e NESL_ServiceSetFullscreen(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(SDL_SetWindowFullscreen(g_service.window, !g_service.fullscreen ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0)) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    g_service.fullscreen = !g_service.fullscreen;

exit:
    return result;
}

bool NESL_ServiceGetButton(nesl_controller_e controller, nesl_button_e button)
{
    static uint32_t KEY[NESL_CONTROLLER_MAX][NESL_BUTTON_MAX] = {
        {
            SDL_SCANCODE_L, SDL_SCANCODE_K, SDL_SCANCODE_C, SDL_SCANCODE_SPACE,
            SDL_SCANCODE_W, SDL_SCANCODE_S, SDL_SCANCODE_A, SDL_SCANCODE_D,
        },
        {
            SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN,
            SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN, SDL_SCANCODE_UNKNOWN,
        },
    };

    return SDL_GetKeyboardState(NULL)[KEY[controller][button]] ? true : false;
}

nesl_error_e NESL_ServiceInit(const char *title, int fullscreen, int linear, int scale)
{
    nesl_error_e result = NESL_SUCCESS;

    if(scale < 1) {
        scale = 1;
    } else if(scale > 4) {
        scale = 4;
    }

    if(SDL_Init(SDL_INIT_AUDIO | SDL_INIT_VIDEO)) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(!(g_service.window = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256 * scale, 240 * scale, SDL_WINDOW_RESIZABLE))) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(!(g_service.renderer = SDL_CreateRenderer(g_service.window, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC))) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_RenderSetLogicalSize(g_service.renderer, 256, 240)) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_SetRenderDrawColor(g_service.renderer, 0, 0, 0, 0)) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_SetHint(SDL_HINT_RENDER_VSYNC, "1") == SDL_FALSE) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, linear ? "1" : "0") == SDL_FALSE) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(!(g_service.texture = SDL_CreateTexture(g_service.renderer, SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_STREAMING, 256, 240))) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(fullscreen) {

        if((result = NESL_ServiceSetFullscreen()) == NESL_FAILURE) {
            goto exit;
        }
    }

    if((result = NESL_ServiceReset()) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

nesl_error_e NESL_ServicePoll(void)
{
    SDL_Event event;
    nesl_error_e result = NESL_SUCCESS;

    while(SDL_PollEvent(&event)) {

        switch(event.type) {
            case SDL_KEYUP:

                if(!event.key.repeat) {

                    switch(event.key.keysym.scancode) {
                        case SDL_SCANCODE_F11:

                            if((result = NESL_ServiceSetFullscreen()) == NESL_FAILURE) {
                                goto exit;
                            }
                            break;
                        case SDL_SCANCODE_R:

                            if((result = NESL_BusInterrupt(NESL_INTERRUPT_RESET)) == NESL_FAILURE) {
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

nesl_error_e NESL_ServiceRedraw(void)
{
    uint32_t elapsed;
    nesl_error_e result = NESL_SUCCESS;

    if(SDL_UpdateTexture(g_service.texture, NULL, (uint32_t *)g_service.pixel, 256 * sizeof(uint32_t))) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_RenderClear(g_service.renderer)) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if(SDL_RenderCopy(g_service.renderer, g_service.texture, NULL, NULL)) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    if((elapsed = (SDL_GetTicks() - g_service.tick)) < (1000 / (float)60)) {
        SDL_Delay((1000 / (float)60) - elapsed);
    }

    SDL_RenderPresent(g_service.renderer);
    g_service.tick = SDL_GetTicks();

exit:
    return result;
}

nesl_error_e NESL_ServiceReset(void)
{
    nesl_error_e result;

    if((result = NESL_ServiceClear()) == NESL_FAILURE) {
        goto exit;
    }

    NESL_ServiceCloseAudio();
    g_service.tick = 0;

exit:
    return result;
}

nesl_error_e NESL_ServiceSetAudio(NESL_ServiceGetAudio callback, void *context)
{
    nesl_error_e result = NESL_SUCCESS;
    SDL_AudioSpec desired = {}, obtained = {};

    desired.callback = callback;
    desired.channels = 1;
    desired.format = AUDIO_F32SYS;
    desired.freq = 44100;
    desired.samples = 1024;
    desired.userdata = context;
    NESL_ServiceCloseAudio();

    if((g_service.audio = SDL_OpenAudioDevice(NULL, 0, &desired, &obtained, 0)) <= 0) {
        result = NESL_SET_ERROR("%s", SDL_GetError());
        goto exit;
    }

    SDL_PauseAudioDevice(g_service.audio, 0);

exit:
    return result;
}

void NESL_ServiceSetPixel(uint8_t color, bool red, bool green, bool blue, uint8_t x, uint8_t y)
{
    static const uint32_t PALETTE[] = {
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

void NESL_ServiceUninit(void)
{
    NESL_ServiceCloseAudio();

    if(g_service.texture) {
        SDL_DestroyTexture(g_service.texture);
    }

    if(g_service.renderer) {
        SDL_DestroyRenderer(g_service.renderer);
    }

    if(g_service.window) {
        SDL_DestroyWindow(g_service.window);
    }

    SDL_Quit();
    memset(&g_service, 0, sizeof(g_service));
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
