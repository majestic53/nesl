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
 * @file NESL_audio.h
 * @brief Audio subsystem.
 */

#ifndef NESL_AUDIO_H_
#define NESL_AUDIO_H_

#include "./audio/NESL_audio_square.h"
#include "../NESL_bus.h"

/*typedef union {

    struct {

        union {

            struct {
                uint8_t index : 4;
                uint8_t unused : 2;
                uint8_t loop : 1;
                uint8_t interrupt_enable : 1;
            };

            uint8_t raw;
        } control;

        union {

            struct {
                uint8_t counter : 7;
            };

            uint8_t raw;
        } load;

        uint8_t address;
        uint8_t length;
    };

    uint8_t byte[4];
} nesl_audio_dmc_t;

typedef union {

    struct {

        union {

            struct {
                uint8_t volume : 4;
                uint8_t volume_const : 1;
                uint8_t loop : 1;
            };

            uint8_t raw;
        } envelope;

        uint8_t unused;

        union {

            struct {
                uint8_t index : 4;
                uint8_t unused : 3;
                uint8_t loop : 1;
            };

            uint8_t raw;
        } period;

        union {

            struct {
                uint8_t unused : 3;
                uint8_t index : 5;
            };

            uint8_t raw;
        } length;
    };

    uint8_t byte[4];
} nesl_audio_noise_t;

typedef union {

    struct {

        union {

            struct {
                uint8_t counter : 7;
                uint8_t control : 1;
            };

            uint8_t raw;
        } control;

        uint8_t unused;
        uint8_t period_low;

        union {

            struct {
                uint8_t period_high : 3;
                uint8_t counter : 5;
            };

            uint8_t raw;
        } length;
    };

    uint8_t byte[4];
} nesl_audio_triangle_t;*/

/**
 * @union nesl_audio_frame_t
 * @brief Audio frame register.
 */
typedef union {

    struct {
        uint8_t unused : 6;                             /*< Unused */
        uint8_t interrupt_disable : 1;                  /*< Interrupt disable flag */
        uint8_t mode : 1;                               /*< 4/5-step mode */
    };

    uint8_t raw;                                        /*< Raw byte */
} nesl_audio_frame_t;

/**
 * @union nesl_audio_status_t
 * @brief Audio status register.
 */
typedef union {

    struct {
        uint8_t square_0 : 1;                           /*< Square-wave flag (channel 1) */
        uint8_t square_1 : 1;                           /*< Square-wave flag (channel 2) */
        uint8_t triangle : 1;                           /*< Triangle-wave flag */
        uint8_t noise : 1;                              /*< Noise-wave flag */
        uint8_t dmc : 1;                                /*< DMC flag */
        uint8_t unused : 1;                             /*< Unused */
        uint8_t frame_interrupt : 1;                    /*< Flag interrupt flag */
        uint8_t dmc_interrupt : 1;                      /*< DMC interrupt flag */
    };

    uint8_t raw;                                        /*< Raw byte */
} nesl_audio_status_t;

/**
 * @struct nesl_audio_t
 * @brief Audio subsystem context.
 */
typedef struct {
    nesl_audio_buffer_t buffer;                         /*< Audio buffer context */
    nesl_audio_frame_t frame;                           /*< Frame register */
    nesl_audio_status_t status;                         /*< Status register */

    struct {
        nesl_audio_square_t square[NESL_CHANNEL_MAX];   /*< Square-wave synthesizer contexts */
        /*nesl_audio_triangle_t triangle;
        nesl_audio_noise_t noise;
        nesl_audio_dmc_t dmc;*/
    } synthesizer;
} nesl_audio_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Cycle audio subsystem through one cycle.
 * @param audio Pointer to audio subsystem context
 * @param cycle Current cycle
 */
void NESL_AudioCycle(nesl_audio_t *audio, uint64_t cycle);

/**
 * @brief Initialize audio subsystem.
 * @param audio Pointer to audio subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_AudioInit(nesl_audio_t *audio);

/**
 * @brief Read byte from audio subsystem.
 * @param audio Pointer to audio subsystem context
 * @param address Audio address
 * @return Audio data
 */
uint8_t NESL_AudioRead(nesl_audio_t *audio, uint16_t address);

/**
 * @brief Reset audio subsystem.
 * @param audio Pointer to audio subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_AudioReset(nesl_audio_t *audio);

/**
 * @brief Uninitialize audio subsystem.
 * @param audio Pointer to audio subsystem context
 */
void NESL_AudioUninit(nesl_audio_t *audio);

/**
 * @brief Write byte to audio subsystem.
 * @param audio Pointer to audio subsystem context
 * @param address Audio address
 * @param data Audio data
 */
void NESL_AudioWrite(nesl_audio_t *audio, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_AUDIO_H_ */
