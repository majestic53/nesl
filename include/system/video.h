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
 * @file video.h
 * @brief Video subsystem.
 */

#ifndef NESL_VIDEO_H_
#define NESL_VIDEO_H_

#include <bus.h>

/**
 * @enum nesl_port_e
 * @brief Video port types.
 */
typedef enum {
    PORT_CONTROL = 0,                               /*< Control register */
    PORT_MASK,                                      /*< Mask register */
    PORT_STATUS,                                    /*< Status register */
    PORT_OAM_ADDRESS,                               /*< OAM address register */
    PORT_OAM_DATA,                                  /*< OAM data register */
    PORT_SCROLL,                                    /*< Scroll register */
    PORT_ADDRESS,                                   /*< Address register */
    PORT_DATA,                                      /*< Data register */
} nesl_port_e;

/**
 * @union nesl_video_address_t
 * @brief Video address register.
 */
typedef union {

    struct {
        uint16_t coarse_x : 5;                      /*< Coarse x-coordinate */
        uint16_t coarse_y : 5;                      /*< Coarse y-coordinate */
        uint16_t nametable_x : 1;                   /*< Nametable x-coordinate */
        uint16_t nametable_y : 1;                   /*< Nametable y-coordinate */
        uint16_t fine_y : 3;                        /*< Fine y-coordinate */
        uint16_t unused : 1;                        /*< Unused bit */
    };

    struct {
        uint8_t low;                                /*< Low byte */
        uint8_t high;                               /*< High byte */
    };

    uint16_t word;                                  /*< Word */
} nesl_video_address_t;

/**
 * @union nesl_video_object_t
 * @brief Video object register.
 */
typedef union {

    struct {
        uint8_t y;                                  /*< Y-coordinate */

        union {

            struct {
                uint8_t bank : 1;                   /*< Pattern bank */
                uint8_t index : 6;                  /*< Pattern index */
            };

            uint8_t raw;                            /*< Raw byte */
        } type;

        union {

            struct {
                uint8_t palette : 2;                /*< Palette [0-3]+4 */
                uint8_t unused : 3;                 /*< Unused bits */
                uint8_t priority : 1;               /*< Priority above background */
                uint8_t flip_horizontal: 1;         /*< Flip horizontally */
                uint8_t flip_vertical: 1;           /*< Flip vertically */
            };

            uint8_t raw;                            /*< Raw byte */
        } attribute;

        uint8_t x;                                  /*< X-coordinate */
    };

    uint8_t byte[4];                                /*< Raw bytes */
    uint32_t raw;                                   /*< Raw dword */
} nesl_video_object_t;

/**
 * @union nesl_video_status_t
 * @brief Video status register.
 */
typedef union {

    struct {
        uint8_t unused : 5;                         /*< Unused bits */
        uint8_t sprite_overflow : 1;                /*< Sprite overflow flag */
        uint8_t sprite_0_hit : 1;                   /*< Sprite zero-hit flag */
        uint8_t vertical_blank : 1;                 /*< Vertical blank flag */
    };

    uint8_t raw;                                    /*< Raw byte */
} nesl_video_status_t;

/**
 * @struct nesl_video_t
 * @brief Video subsystem context.
 */
typedef struct {
    uint16_t cycle;                                 /*< Current cycle (x-coordinate) */
    int16_t scanline;                               /*< Current scanline (y-coordinate) */
    const nesl_mirror_e *mirror;                    /*< Constant pointer to mapper mirror */

    struct {
        nesl_video_address_t v;                     /*< Internal address */
        nesl_video_address_t t;                     /*< External address */
        uint8_t fine_x;                             /*< Fine x-coordinate */
    } address;

    struct {
        uint8_t type;                               /*< Background nametable */

        struct {
            uint8_t data;                           /*< Attribute data */
            nesl_register_t lsb;                    /*< Lower attribute shift-register */
            nesl_register_t msb;                    /*< Upper attribute shift-register */
        } attribute;

        struct {
            nesl_register_t data;                   /*< Pattern data */
            nesl_register_t lsb;                    /*< Lower pattern shift-register */
            nesl_register_t msb;                    /*< Upper pattern shift-register */
        } pattern;
    } background;

    struct {
        bool latch; /*< Data latch */

        union {

            struct {
                uint8_t nametable_x : 1;            /*< Nametable x-coordinate */
                uint8_t nametable_y : 1;            /*< Nametable y-coordinate */
                uint8_t increment : 1;              /*< Increment (0:+1, 1:+31) */
                uint8_t sprite_pattern : 1;         /*< Sprite pattern */
                uint8_t background_pattern : 1;     /*< Background pattern */
                uint8_t sprite_size : 1;            /*< Sprite size */
                uint8_t unused : 1;                 /*< Unused bit */
                uint8_t interrupt : 1;              /*< Interrupt enable flag */
            };

            uint8_t raw;                            /*< Raw byte */
        } control;

        union {

            struct {
                uint8_t greyscale : 1;              /*< Enable greyscale */
                uint8_t background_left_show : 1;   /*< Show background left-most 8-pixels */
                uint8_t sprite_left_show : 1;       /*< Show sprite left-most 8-pixels */
                uint8_t background_show : 1;        /*< Show background */
                uint8_t sprite_show : 1;            /*< Show sprite */
                uint8_t red_emphasis : 1;           /*< Emphasize red channel */
                uint8_t green_emphasis : 1;         /*< Emphasize green channel */
                uint8_t blue_emphasis : 1;          /*< Emphasize blue channel */
            };

            uint8_t raw;                            /*< Raw byte */
        } mask;

        nesl_video_status_t status;                 /*< Status register */
        nesl_register_t oam_address;                /*< OAM address register */
        nesl_register_t data;                       /*< Data register */
    } port;

    struct {
        uint8_t nametable[2][1024];                 /*< Nametables RAM buffers */
        uint8_t palette[32];                        /*< Palette RAM buffer */
        nesl_video_object_t oam[64];                /*< OAM RAM buffer */
    } ram;

    struct {
        uint8_t count;                              /*< Count in scanline */
        bool sprite_0_found;                        /*< Sprite zero found flag */
        bool sprite_0_render;                       /*< Sprite zero render flag */
        nesl_video_object_t object[8];              /*< Sprites to render */

        struct {
            uint8_t lsb;                            /*< Lower sprite pattern shift-register */
            uint8_t msb;                            /*< Upper sprite pattern shift-register */
        } pattern[8];
    } sprite;
} nesl_video_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Cycle video subsystem through one cycle.
 * @param video Pointer to video subsystem context
 */
bool nesl_video_cycle(nesl_video_t *video);

/**
 * @brief Initialize video subsystem.
 * @param video Pointer to video subsystem context
 * @param mirror Constant pointer to mapper mirror
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_video_initialize(nesl_video_t *video, const nesl_mirror_e *mirror);

/**
 * @brief Read byte from video subsystem.
 * @param video Pointer to video subsystem context
 * @param address Video address
 * @return Video data
 */
uint8_t nesl_video_read(nesl_video_t *video, uint16_t address);

/**
 * @brief Read byte from video subsystem OAM.
 * @param video Pointer to video subsystem context
 * @param address Video address
 * @return Video data
 */
uint8_t nesl_video_read_oam(nesl_video_t *video, uint8_t address);

/**
 * @brief Read byte from video subsystem port.
 * @param video Pointer to video subsystem context
 * @param address Video address
 * @return Video data
 */
uint8_t nesl_video_read_port(nesl_video_t *video, uint16_t address);

/**
 * @brief Reset video subsystem.
 * @param video Pointer to video subsystem context
 * @param mirror Constant pointer to mapper mirror
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_video_reset(nesl_video_t *video, const nesl_mirror_e *mirror);

/**
 * @brief Uninitialize video subsystem.
 * @param video Pointer to video subsystem context
 */
void nesl_video_uninitialize(nesl_video_t *video);

/**
 * @brief Write byte to video subsystem.
 * @param video Pointer to video subsystem context
 * @param address Video address
 * @param data Video data
 */
void nesl_video_write(nesl_video_t *video, uint16_t address, uint8_t data);

/**
 * @brief Write byte to video subsystem OAM.
 * @param video Pointer to video subsystem context
 * @param address Video address
 * @param data Video data
 */
void nesl_video_write_oam(nesl_video_t *video, uint8_t address, uint8_t data);

/**
 * @brief Write byte to video subsystem port.
 * @param video Pointer to video subsystem context
 * @param address Video address
 * @param data Video data
 */
void nesl_video_write_port(nesl_video_t *video, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_VIDEO_H_ */
