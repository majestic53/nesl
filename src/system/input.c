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

#include "../../include/system/input.h"
#include "../../include/service.h"

typedef union {

    struct {
        uint8_t state : 1;
    };

    uint8_t raw;
} nesl_button_t;

typedef union {

    struct {
        uint8_t state : 1;
    };

    uint8_t raw;
} nesl_stobe_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int nesl_input_initialize(nesl_input_t *input)
{

    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {
        input->button[controller].position = NESL_BUTTON_MAX;
    }

    return NESL_SUCCESS;
}

uint8_t nesl_input_read(nesl_input_t *input, uint16_t address)
{
    int controller = NESL_CONTROLLER_MAX;
    nesl_button_t result = { .raw = 0x41 };

    switch(address) {
        case 0x4016:
            controller = NESL_CONTROLLER_1;
            break;
        case 0x4017:
            controller = NESL_CONTROLLER_2;
            break;
        default:
            break;
    }

    if(controller < NESL_CONTROLLER_MAX) {

        if(input->button[controller].position < NESL_BUTTON_MAX) {
            result.state = input->button[controller].state[input->button[controller].position++] ? 1 : 0;
        }
    }

    return result.raw;
}

int nesl_input_reset(nesl_input_t *input)
{

    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

        for(int button = 0; button < NESL_BUTTON_MAX; ++button) {
            input->button[controller].state[button] = false;
        }

        input->button[controller].position = NESL_BUTTON_MAX;
    }

    input->strobe = false;

    return NESL_SUCCESS;
}

void nesl_input_uninitialize(nesl_input_t *input)
{
    memset(input, 0, sizeof(*input));
}

void
nesl_input_write(nesl_input_t *input, uint16_t address, uint8_t data)
{
    nesl_stobe_t strobe = {};

    switch(address) {
        case 0x4016:
            strobe.raw = data;

            if(input->strobe) {

                if(!strobe.state) {

                    for(int controller = 0; controller < NESL_CONTROLLER_MAX; ++controller) {

                        for(int button = 0; button < NESL_BUTTON_MAX; ++button) {
                            input->button[controller].state[button] = nesl_service_button(controller, button);
                        }

                        input->button[controller].position = NESL_BUTTON_A;
                    }
                }
            }

            input->strobe = strobe.state;
            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
