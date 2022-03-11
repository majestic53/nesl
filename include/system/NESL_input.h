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
 * @file NESL_input.h
 * @brief Input subsystem.
 */

#ifndef NESL_INPUT_H_
#define NESL_INPUT_H_

#include <NESL_bus.h>

/**
 * @struct nesl_input_t
 * @brief Input context.
 */
typedef struct {

    struct {
        int position;                   /*< Button position */
        bool state[NESL_BUTTON_MAX];    /*< Button state (true:pressed, false:released) */
    } button[NESL_CONTROLLER_MAX];

    bool strobe;                        /*< Strobe state */
} nesl_input_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Initialize input subsystem.
 * @param input Pointer to input subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_InputInitialize(nesl_input_t *input);

/**
 * @brief Read byte from input subsystem.
 * @param input Pointer to input subsystem context
 * @param address Input address
 * @return Input data
 */
uint8_t NESL_InputRead(nesl_input_t *input, uint16_t address);

/**
 * @brief Reset input subsystem.
 * @param input Pointer to input subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_InputReset(nesl_input_t *input);

/**
 * @brief Uninitialize input subsystem.
 * @param input Pointer to input subsystem context
 */
void NESL_InputUninitialize(nesl_input_t *input);

/**
 * @brief Write byte to input subsystem.
 * @param input Pointer to input subsystem context
 * @param address Input address
 * @param data Input data
 */
void NESL_InputWrite(nesl_input_t *input, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_INPUT_H_ */
