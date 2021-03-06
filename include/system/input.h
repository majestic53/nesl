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
 * @file input.h
 * @brief Input subsystem.
 */

#ifndef NESL_INPUT_H_
#define NESL_INPUT_H_

#include <bus.h>

/*!
 * @union nesl_input_state_t
 * @brief Input state context.
 */
typedef union {

    struct {
        uint8_t button : 1;                     /*!< Button state */
        uint8_t unused : 2;                     /*!< Unused bits */
        uint8_t sensor : 1;                     /*!< Sensor state */
        uint8_t trigger : 1;                    /*!< Trigger state */
    };

    uint8_t raw;                                /*!< Raw state */
} nesl_input_state_t;

/*!
 * @struct nesl_input_t
 * @brief Input context.
 */
typedef struct {

    struct {
        int position;                           /*!< Button position */
        nesl_input_state_t state[BUTTON_MAX];   /*!< Button state */
    } controller;

    nesl_input_state_t zapper;                  /*!< Zapper (light-gun) */
    bool strobe;                                /*!< Strobe state */
} nesl_input_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Initialize input subsystem.
 * @param[in,out] input Pointer to input subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_input_initialize(nesl_input_t *input);

/*!
 * @brief Read byte from input subsystem.
 * @param[in,out] input Pointer to input subsystem context
 * @param[in] address Input address
 * @return Input data
 */
uint8_t nesl_input_read(nesl_input_t *input, uint16_t address);

/*!
 * @brief Reset input subsystem.
 * @param[in,out] input Pointer to input subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_input_reset(nesl_input_t *input);

/*!
 * @brief Uninitialize input subsystem.
 * @param[in,out] input Pointer to input subsystem context
 */
void nesl_input_uninitialize(nesl_input_t *input);

/*!
 * @brief Write byte to input subsystem.
 * @param[in,out] input Pointer to input subsystem context
 * @param[in] address Input address
 * @param[in] data Input data
 */
void nesl_input_write(nesl_input_t *input, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_INPUT_H_ */
