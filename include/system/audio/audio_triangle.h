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
 * @file audio_triangle.h
 * @brief Audio triangle-wave synthesizer.
 */

#ifndef NESL_AUDIO_TRIANGLE_H_
#define NESL_AUDIO_TRIANGLE_H_

#include <audio_buffer.h>

/*!
 * @struct nesl_audio_triangle_t
 * @brief Audio triangle-wave synthesizer context.
 */
typedef struct {
    nesl_audio_buffer_t buffer;                 /*!< Audio buffer context */

    union {

        struct {

            union {

                struct {
                    uint8_t counter : 7;        /*!< Counter */
                    uint8_t control : 1;        /*!< Control flag */
                };

                uint8_t raw;                    /*!< Raw byte */
            } control;

            uint8_t unused;                     /*!< Unused register */
            uint8_t period_low;                 /*!< Low period register */

            union {

                struct {
                    uint8_t period_high : 3;    /*!< High period */
                    uint8_t counter : 5;        /*!< Counter */
                };

                uint8_t raw;                    /*!< Raw byte */
            } length;
        };

        uint8_t byte[4];                            /*!< Raw bytes */
    } state;
} nesl_audio_triangle_t;

/*!
 * @brief Cycle audio triangle-wave synthesizer through one cycle.
 * @param[in,out] triangle Pointer to audio triangle-wave synthesizer context
 * @param elapsed Current time in seconds
 */
void nesl_audio_triangle_cycle(nesl_audio_triangle_t *triangle, float elapsed);

/*!
 * @brief Initialize audio triangle-wave synthesizer.
 * @param[in,out] triangle Pointer to audio triangle-wave synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_audio_triangle_initialize(nesl_audio_triangle_t *triangle);

/*!
 * @brief Read bytes from audio triangle-wave synthesizer.
 * @param[in,out] triangle Pointer to audio triangle-wave synthesizer context
 * @param[in,out] data Pointer to data array
 * @param[in] length Maximum number of entries in data array
 * @return Number of entries read
 */
int nesl_audio_triangle_read(nesl_audio_triangle_t *triangle, float *data, int length);

/*!
 * @brief Readable bytes in audio triangle-wave synthesizer.
 * @param[in,out] triangle Pointer to audio triangle-wave synthesizer context
 * @return Number of entries readable
 */
int nesl_audio_triangle_readable(nesl_audio_triangle_t *triangle);

/*!
 * @brief Reset audio triangle-wave synthesizer.
 * @param[in,out] triangle Pointer to audio triangle-wave synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_audio_triangle_reset(nesl_audio_triangle_t *triangle);

/*!
 * @brief Uninitialize audio triangle-wave synthesizer.
 * @param[in,out] triangle Pointer to audio triangle-wave synthesizer context
 */
void nesl_audio_triangle_uninitialize(nesl_audio_triangle_t *triangle);

/*!
 * @brief Write byte to audio triangle-wave synthesizer.
 * @param[in,out] triangle Pointer to audio triangle-wave synthesizer context
 * @param[in] address Audio address
 * @param[in] data Audio data
 */
void nesl_audio_triangle_write(nesl_audio_triangle_t *triangle, uint16_t address, uint8_t data);

#endif /* NESL_AUDIO_TRIANGLE_H_ */
