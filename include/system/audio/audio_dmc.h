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
 * @file audio_dmc.h
 * @brief Audio DMC synthesizer.
 */

#ifndef NESL_AUDIO_DMC_H_
#define NESL_AUDIO_DMC_H_

#include <audio_buffer.h>

/*!
 * @struct nesl_audio_dmc_t
 * @brief Audio DMC synthesizer context.
 */
typedef struct {
    nesl_audio_buffer_t buffer;                     /*!< Audio buffer context */

    union {

        struct {

            union {

                struct {
                    uint8_t index : 4;              /*!< Index */
                    uint8_t unused : 2;             /*!< Unused bits */
                    uint8_t loop : 1;               /*!< Loop flag */
                    uint8_t interrupt_enable : 1;   /*!< Interrupt enable flag */
                };

                uint8_t raw;                        /*!< Raw byte */
            } control;

            union {

                struct {
                    uint8_t counter : 7;            /*!< Counter */
                };

                uint8_t raw;                        /*!< Raw byte */
            } load;

            uint8_t address;                        /*!< Address register */
            uint8_t length;                         /*!< Length register */
        };

        uint8_t byte[4];                            /*!< Raw bytes */
    } state;
} nesl_audio_dmc_t;

/*!
 * @brief Cycle audio DMC synthesizer through one cycle.
 * @param[in,out] dmc Pointer to audio DMC synthesizer context
 * @param[in] elapsed Current time in seconds
 */
void nesl_audio_dmc_cycle(nesl_audio_dmc_t *dmc, float elapsed);

/*!
 * @brief Initialize audio DMC synthesizer.
 * @param[in,out] dmc Pointer to audio DMC synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_audio_dmc_initialize(nesl_audio_dmc_t *dmc);

/*!
 * @brief Read bytes from audio DMC synthesizer.
 * @param[in,out] dmc Pointer to audio DMC synthesizer context
 * @param[in,out] data Pointer to data array
 * @param[in] length Maximum number of entries in data array
 * @return Number of entries read
 */
int nesl_audio_dmc_read(nesl_audio_dmc_t *dmc, float *data, int length);

/*!
 * @brief Readable bytes in audio DMC synthesizer.
 * @param[in,out] dmc Pointer to audio DMC synthesizer context
 * @return Number of entries readable
 */
int nesl_audio_dmc_readable(nesl_audio_dmc_t *dmc);

/*!
 * @brief Reset audio DMC synthesizer.
 * @param[in,out] dmc Pointer to audio DMC synthesizer context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_audio_dmc_reset(nesl_audio_dmc_t *dmc);

/*!
 * @brief Uninitialize audio DMC synthesizer.
 * @param[in,out] dmc Pointer to audio DMC synthesizer context
 */
void nesl_audio_dmc_uninitialize(nesl_audio_dmc_t *dmc);

/*!
 * @brief Write byte to audio DMC synthesizer.
 * @param[in,out] dmc Pointer to audio DMC synthesizer context
 * @param[in] address Audio address
 * @param[in] data Audio data
 */
void nesl_audio_dmc_write(nesl_audio_dmc_t *dmc, uint16_t address, uint8_t data);

#endif /* NESL_AUDIO_DMC_H_ */
