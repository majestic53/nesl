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
 * @file NESL_audio_triangle.h
 * @brief Audio triangle-wave synthesizer.
 */

#ifndef NESL_AUDIO_TRIANGLE_H_
#define NESL_AUDIO_TRIANGLE_H_

#include "./NESL_audio_buffer.h"

/**
 * @union nesl_audio_triangle_control_t
 * @brief Audio triangle-wave synthesizer control register.
 */
typedef union {

    struct {
        uint8_t counter : 7;                        /*< Counter */
        uint8_t control : 1;                        /*< Control flag */
    };

    uint8_t raw;                                    /*< Raw byte */
} nesl_audio_triangle_control_t;

/**
 * @union nesl_audio_triangle_length_t
 * @brief Audio triangle-wave synthesizer length register.
 */
typedef union {

    struct {
        uint8_t period_high : 3;                    /*< High period */
        uint8_t counter : 5;                        /*< Counter */
    };

    uint8_t raw;                                    /*< Raw byte */
} nesl_audio_triangle_length_t;

/**
 * @struct nesl_audio_triangle_t
 * @brief Audio triangle-wave synthesizer context.
 */
typedef struct {
    nesl_audio_buffer_t buffer;                     /*< Audio buffer context */

    union {

        struct {
            nesl_audio_triangle_control_t control;  /*< Control register */
            uint8_t unused;                         /*< Unused register */
            uint8_t period_low;                     /*< Low period register */
            nesl_audio_triangle_control_t length;   /*< Length register */
        };

        uint8_t byte[4];                            /*< Raw bytes */
    } state;
} nesl_audio_triangle_t;

/**
 * @brief Cycle audio triangle-wave synthesizer through one cycle.
 * @param triangle Pointer to audio triangle-wave synthesizer context
 */
void NESL_AudioTriangleCycle(nesl_audio_triangle_t *triangle);

/**
 * @brief Initialize audio triangle-wave synthesizer.
 * @param triangle Pointer to audio triangle-wave synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_AudioTriangleInit(nesl_audio_triangle_t *triangle);

/**
 * @brief Read bytes from audio triangle-wave synthesizer.
 * @param triangle Pointer to audio triangle-wave synthesizer context
 * @param data Pointer to data array
 * @param length Maximum number of entries in data array
 * @return Number of entries read
 */
int NESL_AudioTriangleRead(nesl_audio_triangle_t *triangle, float *data, int length);

/**
 * @brief Readable bytes in audio triangle-wave synthesizer.
 * @param triangle Pointer to audio triangle-wave synthesizer context
 * @return Number of entries readable
 */
int NESL_AudioTriangleReadable(nesl_audio_triangle_t *triangle);

/**
 * @brief Reset audio triangle-wave synthesizer.
 * @param triangle Pointer to audio triangle-wave synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_AudioTriangleReset(nesl_audio_triangle_t *triangle);

/**
 * @brief Uninitialize audio triangle-wave synthesizer.
 * @param triangle Pointer to audio triangle-wave synthesizer context
 */
void NESL_AudioTriangleUninit(nesl_audio_triangle_t *triangle);

/**
 * @brief Write byte to audio triangle-wave synthesizer.
 * @param triangle Pointer to audio triangle-wave synthesizer context
 * @param address Audio address
 * @param data Audio data
 */
void NESL_AudioTriangleWrite(nesl_audio_triangle_t *triangle, uint16_t address, uint8_t data);

#endif /* NESL_AUDIO_TRIANGLE_H_ */
