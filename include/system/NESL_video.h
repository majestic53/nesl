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

#ifndef NESL_VIDEO_H_
#define NESL_VIDEO_H_

#include "../NESL_bus.h"

typedef union {

    struct {
        uint16_t coarse_x : 5;
        uint16_t coarse_y : 5;
        uint16_t nametable_x : 1;
        uint16_t nametable_y : 1;
        uint16_t fine_y : 3;
        uint16_t unused : 1;
    };

    struct {
        uint8_t low;
        uint8_t high;
    };

    uint16_t word;
} nesl_video_address_t;

typedef union {

    struct {
        uint8_t y;

        union {

            struct {
                uint8_t bank : 1;
                uint8_t index : 6;
            };

            uint8_t raw;
        } type;

        union {

            struct {
                uint8_t palette : 2;
                uint8_t unused : 3;
                uint8_t priority : 1;
                uint8_t flip_horizontal: 1;
                uint8_t flip_vertical: 1;
            };

            uint8_t raw;
        } attribute;

        uint8_t x;
    };

    uint8_t byte[4];
    uint32_t raw;
} nesl_video_object_t;

typedef union {

    struct {
        uint8_t unused : 5;
        uint8_t sprite_overflow : 1;
        uint8_t sprite_0_hit : 1;
        uint8_t vertical_blank : 1;
    };

    uint8_t raw;
} nesl_video_status_t;

typedef struct {
    uint16_t cycle;
    int16_t scanline;
    const int *mirror;

    struct {
        nesl_video_address_t v;
        nesl_video_address_t t;
        uint8_t fine_x;
    } address;

    struct {
        uint8_t type;

        struct {
            uint8_t data;
            nesl_register_t lsb;
            nesl_register_t msb;
        } attribute;

        struct {
            nesl_register_t data;
            nesl_register_t lsb;
            nesl_register_t msb;
        } pattern;
    } background;

    struct {
        bool latch;

        union {

            struct {
                uint8_t nametable_x : 1;
                uint8_t nametable_y : 1;
                uint8_t increment : 1;
                uint8_t sprite_pattern : 1;
                uint8_t background_pattern : 1;
                uint8_t sprite_size : 1;
                uint8_t select : 1;
                uint8_t interrupt : 1;
            };

            uint8_t raw;
        } control;

        union {

            struct {
                uint8_t greyscale : 1;
                uint8_t background_left_show : 1;
                uint8_t sprite_left_show : 1;
                uint8_t background_show : 1;
                uint8_t sprite_show : 1;
                uint8_t red_emphasis : 1;
                uint8_t green_emphasis : 1;
                uint8_t blue_emphasis : 1;
            };

            uint8_t raw;
        } mask;

        nesl_video_status_t status;
        nesl_register_t oam_address;
        nesl_register_t data;
    } port;

    struct {
        uint8_t nametable[2][1024];
        uint8_t palette[32];
        nesl_video_object_t oam[64];
    } ram;

    struct {
        uint8_t count;
        bool sprite_0_found;
        bool sprite_0_render;
        nesl_video_object_t object[8];

        struct {
            uint8_t lsb;
            uint8_t msb;
        } pattern[8];
    } sprite;
} nesl_video_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool NESL_VideoCycle(nesl_video_t *video);
int NESL_VideoInit(nesl_video_t *video, const int *mirror);
uint8_t NESL_VideoRead(nesl_video_t *video, uint16_t address);
uint8_t NESL_VideoReadOam(nesl_video_t *video, uint8_t address);
uint8_t NESL_VideoReadPort(nesl_video_t *video, uint16_t address);
int NESL_VideoReset(nesl_video_t *video, const int *mirror);
void NESL_VideoUninit(nesl_video_t *video);
void NESL_VideoWrite(nesl_video_t *video, uint16_t address, uint8_t data);
void NESL_VideoWriteOam(nesl_video_t *video, uint8_t address, uint8_t data);
void NESL_VideoWritePort(nesl_video_t *video, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_VIDEO_H_ */
