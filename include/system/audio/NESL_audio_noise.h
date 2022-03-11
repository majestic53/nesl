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
 * @file NESL_audio_noise.h
 * @brief Audio noise synthesizer.
 */

#ifndef NESL_AUDIO_NOISE_H_
#define NESL_AUDIO_NOISE_H_

#include <NESL_audio_buffer.h>

/**
 * @struct nesl_audio_noise_t
 * @brief Audio noise synthesizer context.
 */
typedef struct {
    nesl_audio_buffer_t buffer;                 /*< Audio buffer context */

    union {

        struct {

            union {

                struct {
                    uint8_t volume : 4;         /*< Volume */
                    uint8_t volume_const : 1;   /*< Constant volume */
                    uint8_t loop : 1;           /*< Loop flag */
                };

                uint8_t raw;                    /*< Raw byte */
            } envelope;

            uint8_t unused;                     /*< Unused register */

            union {

                struct {
                    uint8_t index : 4;          /*< Index */
                    uint8_t unused : 3;         /*< Unused bits */
                    uint8_t loop : 1;           /*< Loop flag */
                };

                uint8_t raw;                    /*< Raw byte */
            } period;

            union {

                struct {
                    uint8_t unused : 3;         /*< Unused bits */
                    uint8_t index : 5;          /*< Index */
                };

                uint8_t raw;                    /*< Raw byte */
            } length;
        };

        uint8_t byte[4];                        /*< Raw bytes */
    } state;
} nesl_audio_noise_t;

/**
 * @brief Cycle audio noise synthesizer through one cycle.
 * @param noise Pointer to audio noise synthesizer context
 * @param cycle Current cycle
 */
void NESL_AudioNoiseCycle(nesl_audio_noise_t *noise, uint64_t cycle);

/**
 * @brief Initialize audio noise synthesizer.
 * @param noise Pointer to audio noise synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_AudioNoiseInitialize(nesl_audio_noise_t *noise);

/**
 * @brief Read bytes from audio noise synthesizer.
 * @param noise Pointer to audio noise synthesizer context
 * @param data Pointer to data array
 * @param length Maximum number of entries in data array
 * @return Number of entries read
 */
int NESL_AudioNoiseRead(nesl_audio_noise_t *noise, int16_t *data, int length);

/**
 * @brief Readable bytes in audio noise synthesizer.
 * @param noise Pointer to audio noise synthesizer context
 * @return Number of entries readable
 */
int NESL_AudioNoiseReadable(nesl_audio_noise_t *noise);

/**
 * @brief Reset audio noise synthesizer.
 * @param noise Pointer to audio noise synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_AudioNoiseReset(nesl_audio_noise_t *noise);

/**
 * @brief Uninitialize audio noise synthesizer.
 * @param noise Pointer to audio noise synthesizer context
 */
void NESL_AudioNoiseUninitialize(nesl_audio_noise_t *noise);

/**
 * @brief Write byte to audio noise synthesizer.
 * @param noise Pointer to audio noise synthesizer context
 * @param address Audio address
 * @param data Audio data
 */
void NESL_AudioNoiseWrite(nesl_audio_noise_t *noise, uint16_t address, uint8_t data);

#endif /* NESL_AUDIO_NOISE_H_ */
