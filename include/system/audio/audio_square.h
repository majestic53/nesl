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
 * @file audio_square.h
 * @brief Audio square-wave synthesizer.
 */

#ifndef NESL_AUDIO_SQUARE_H_
#define NESL_AUDIO_SQUARE_H_

#include <audio_buffer.h>

/*!
 * @struct nesl_audio_square_t
 * @brief Audio square-wave synthesizer context.
 */
typedef struct {
    nesl_audio_buffer_t buffer;                 /*!< Audio buffer context */

    union {

        struct {

            union {

                struct {
                    uint8_t volume : 4;         /*!< Volume */
                    uint8_t volume_const : 1;   /*!< Constant volume */
                    uint8_t loop : 1;           /*!< Loop flag */
                    uint8_t duty : 2;           /*!< Duty-cycle */
                };

                uint8_t raw;                    /*!< Raw byte */
            } envelope;

            union {

                struct {
                    uint8_t shift : 3;          /*!< Shift */
                    uint8_t negative : 1;       /*!< Negative flag */
                    uint8_t period : 3;         /*!< Period */
                    uint8_t enable : 1;         /*!< Enable flag */
                };

                uint8_t raw;                    /*!< Raw byte */
            } sweep;

            uint8_t period_low;                 /*!< Low period register */

            union {

                struct {
                    uint8_t period_high : 3;    /*!< High period */
                    uint8_t counter : 5;        /*!< Counter */
                };

                uint8_t raw;                    /*!< Raw byte */
            } length;
        };

        uint8_t byte[4];                        /*!< Raw bytes */
    } state;
} nesl_audio_square_t;

/*!
 * @brief Cycle audio square-wave synthesizer through one cycle.
 * @param[in,out] square Pointer to audio square-wave synthesizer context
 * @param elapsed Current time in seconds
 */
void nesl_audio_square_cycle(nesl_audio_square_t *square, float elapsed);

/*!
 * @brief Initialize audio square-wave synthesizer.
 * @param[in,out] square Pointer to audio square-wave synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_audio_square_initialize(nesl_audio_square_t *square);

/*!
 * @brief Read bytes from audio square-wave synthesizer.
 * @param[in,out] square Pointer to audio square-wave synthesizer context
 * @param[in,out] data Pointer to data array
 * @param[in] length Maximum number of entries in data array
 * @return Number of entries read
 */
int nesl_audio_square_read(nesl_audio_square_t *square, float *data, int length);

/*!
 * @brief Readable bytes in audio square-wave synthesizer.
 * @param[in,out] square Pointer to audio square-wave synthesizer context
 * @return Number of entries readable
 */
int nesl_audio_square_readable(nesl_audio_square_t *square);

/*!
 * @brief Reset audio square-wave synthesizer.
 * @param[in,out] square Pointer to audio square-wave synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_audio_square_reset(nesl_audio_square_t *square);

/*!
 * @brief Uninitialize audio square-wave synthesizer.
 * @param[in,out] square Pointer to audio square-wave synthesizer context
 */
void nesl_audio_square_uninitialize(nesl_audio_square_t *square);

/*!
 * @brief Write byte to audio square-wave synthesizer.
 * @param[in,out] square Pointer to audio square-wave synthesizer context
 * @param[in] address Audio address
 * @param[in] data Audio data
 */
void nesl_audio_square_write(nesl_audio_square_t *square, uint16_t address, uint8_t data);

#endif /* NESL_AUDIO_SQUARE_H_ */
