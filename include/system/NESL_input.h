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

#ifndef NESL_INPUT_H_
#define NESL_INPUT_H_

#include "../NESL_bus.h"

typedef struct {

    struct {
        int position;
        bool state[NESL_BUTTON_MAX];
    } button[NESL_CONTROLLER_MAX];

    bool strobe;
} nesl_input_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_InputInit(nesl_input_t *input);
uint8_t NESL_InputRead(nesl_input_t *input, uint16_t address);
int NESL_InputReset(nesl_input_t *input);
void NESL_InputUninit(nesl_input_t *input);
void NESL_InputWrite(nesl_input_t *input, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_INPUT_H_ */
