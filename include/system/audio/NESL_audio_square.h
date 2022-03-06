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
 * @file NESL_audio_square.h
 * @brief Audio square-wave synthesizer.
 */

#ifndef NESL_AUDIO_SQUARE_H_
#define NESL_AUDIO_SQUARE_H_

#include "./NESL_audio_buffer.h"

/**
 * @union nesl_audio_square_envelope_t
 * @brief Audio square-wave synthesizer envelope register.
 */
typedef union {

    struct {
        uint8_t volume : 4;                         /*< Volume */
        uint8_t volume_const : 1;                   /*< Constant volume */
        uint8_t loop : 1;                           /*< Loop flag */
        uint8_t duty : 2;                           /*< Duty-cycle */
    };

    uint8_t raw;                                    /*< Raw byte */
} nesl_audio_square_envelope_t;

/**
 * @union nesl_audio_square_length_t
 * @brief Audio square-wave synthesizer length register.
 */
typedef union {

    struct {
        uint8_t period_high : 3;                    /*< High period */
        uint8_t counter : 5;                        /*< Counter */
    };

    uint8_t raw;                                    /*< Raw byte */
} nesl_audio_square_length_t;

/**
 * @union nesl_audio_square_sweep_t
 * @brief Audio square-wave synthesizer sweep register.
 */
typedef union {

    struct {
        uint8_t shift : 3;                          /*< Shift */
        uint8_t negative : 1;                       /*< Negative flag */
        uint8_t period : 3;                         /*< Period */
        uint8_t enable : 1;                         /*< Enable flag */
    };

    uint8_t raw;                                    /*< Raw byte */
} nesl_audio_square_sweep_t;

/**
 * @struct nesl_audio_square_t
 * @brief Audio square-wave synthesizer context.
 */
typedef struct {
    nesl_audio_buffer_t buffer;                     /*< Audio buffer context */

    union {

        struct {
            nesl_audio_square_envelope_t envelope;  /*< Envelope register */
            nesl_audio_square_sweep_t sweep;        /*< Sweep register */
            uint8_t period_low;                     /*< Low period register */
            nesl_audio_square_length_t length;      /*< Length register */
        };

        uint8_t byte[4];                            /*< Raw bytes */
    } state;
} nesl_audio_square_t;

/**
 * @brief Cycle audio square-wave synthesizer through one cycle.
 * @param square Pointer to audio square-wave synthesizer context
 */
void NESL_AudioSquareCycle(nesl_audio_square_t *square);

/**
 * @brief Initialize audio square-wave synthesizer.
 * @param square Pointer to audio square-wave synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_AudioSquareInit(nesl_audio_square_t *square);

/**
 * @brief Read bytes from audio square-wave synthesizer.
 * @param square Pointer to audio square-wave synthesizer context
 * @param data Pointer to data array
 * @param length Maximum number of entries in data array
 * @return Number of entries read
 */
int NESL_AudioSquareRead(nesl_audio_square_t *square, float *data, int length);

/**
 * @brief Readable bytes in audio square-wave synthesizer.
 * @param square Pointer to audio square-wave synthesizer context
 * @return Number of entries readable
 */
int NESL_AudioSquareReadable(nesl_audio_square_t *square);

/**
 * @brief Reset audio square-wave synthesizer.
 * @param square Pointer to audio square-wave synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_AudioSquareReset(nesl_audio_square_t *square);

/**
 * @brief Uninitialize audio square-wave synthesizer.
 * @param square Pointer to audio square-wave synthesizer context
 */
void NESL_AudioSquareUninit(nesl_audio_square_t *square);

/**
 * @brief Write byte to audio square-wave synthesizer.
 * @param square Pointer to audio square-wave synthesizer context
 * @param address Audio address
 * @param data Audio data
 */
void NESL_AudioSquareWrite(nesl_audio_square_t *square, uint16_t address, uint8_t data);

#endif /* NESL_AUDIO_SQUARE_H_ */
