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
 * @file audio.h
 * @brief Audio subsystem.
 */

#ifndef NESL_AUDIO_H_
#define NESL_AUDIO_H_

#include <audio_dmc.h>
#include <audio_noise.h>
#include <audio_square.h>
#include <audio_triangle.h>
#include <bus.h>

/*!
 * @enum nesl_synthesizer_e
 * @brief Synthesizer type.
 */
typedef enum {
    SYNTHESIZER_SQUARE_1 = 0,                                       /*!< First square-wave synthesizer */
    SYNTHESIZER_SQUARE_2,                                           /*!< Second square-wave synthesizer */
    SYNTHESIZER_TRIANGLE,                                           /*!< Triangle-wave synthesizer */
    SYNTHESIZER_NOISE,                                              /*!< Noise synthesizer */
    SYNTHESIZER_DMC,                                                /*!< DMC synthesizer */
    SYNTHESIZER_MAX,                                                /*!< Maximum synthesizer */
} nesl_synthesizer_e;

/*!
 * @union nesl_audio_status_t
 * @brief Audio status register.
 */
typedef union {

    struct {
        uint8_t square_0 : 1;                                       /*!< Square-wave flag (channel 1) */
        uint8_t square_1 : 1;                                       /*!< Square-wave flag (channel 2) */
        uint8_t triangle : 1;                                       /*!< Triangle-wave flag */
        uint8_t noise : 1;                                          /*!< Noise-wave flag */
        uint8_t dmc : 1;                                            /*!< DMC flag */
        uint8_t unused : 1;                                         /*!< Unused flag */
        uint8_t frame_interrupt : 1;                                /*!< Flag interrupt flag */
        uint8_t dmc_interrupt : 1;                                  /*!< DMC interrupt flag */
    };

    uint8_t raw;                                                    /*!< Raw byte */
} nesl_audio_status_t;

/*!
 * @struct nesl_audio_t
 * @brief Audio subsystem context.
 */
typedef struct {
    nesl_audio_buffer_t buffer;                                     /*!< Audio buffer context */
    nesl_audio_status_t status;                                     /*!< Status register */

    union {

        struct {
            uint8_t unused : 6;                                     /*!< Unused bits */
            uint8_t interrupt_disable : 1;                          /*!< Interrupt disable flag */
            uint8_t mode : 1;                                       /*!< 4/5-step mode */
        };

        uint8_t raw;                                                /*!< Raw byte */
    } frame;

    struct {
        nesl_audio_square_t square[SYNTHESIZER_SQUARE_2 + 1];  /*!< Square-wave synthesizer contexts */
        nesl_audio_triangle_t triangle;                             /*!< Triangle synthesizer context */
        nesl_audio_noise_t noise;                                   /*!< Noise synthesizer context */
        nesl_audio_dmc_t dmc;                                       /*!< DMC synthesizer contexts */
    } synthesizer;
} nesl_audio_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Cycle audio subsystem through one cycle.
 * @param[in,out] audio Pointer to audio subsystem context
 * @param cycle Current cycle
 */
void nesl_audio_cycle(nesl_audio_t *audio, uint64_t cycle);

/*!
 * @brief Initialize audio subsystem.
 * @param[in,out] audio Pointer to audio subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_audio_initialize(nesl_audio_t *audio);

/*!
 * @brief Read byte from audio subsystem.
 * @param[in,out] audio Pointer to audio subsystem context
 * @param[in] address Audio address
 * @return Audio data
 */
uint8_t nesl_audio_read(nesl_audio_t *audio, uint16_t address);

/*!
 * @brief Reset audio subsystem.
 * @param[in,out] audio Pointer to audio subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_audio_reset(nesl_audio_t *audio);

/*!
 * @brief Uninitialize audio subsystem.
 * @param[in,out] audio Pointer to audio subsystem context
 */
void nesl_audio_uninitialize(nesl_audio_t *audio);

/*!
 * @brief Write byte to audio subsystem.
 * @param[in,out] audio Pointer to audio subsystem context
 * @param[in] address Audio address
 * @param[in] data Audio data
 */
void nesl_audio_write(nesl_audio_t *audio, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_AUDIO_H_ */
