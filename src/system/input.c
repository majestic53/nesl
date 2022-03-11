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
 * @file input.c
 * @brief Input subsystem.
 */

#include <input.h>

/**
 * @struct nesl_strobe_t
 * @brief Input strobe state.
 */
typedef union {

    struct {
        uint8_t state : 1;  /*< Strobe state */
    };

    uint8_t raw;            /*< Raw byte */
} nesl_strobe_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_error_e nesl_input_initialize(nesl_input_t *input)
{
    return nesl_input_reset(input);
}

uint8_t nesl_input_read(nesl_input_t *input, uint16_t address)
{
    nesl_strobe_t result = { .raw = 0x41 };
    nesl_controller_e controller = CONTROLLER_MAX;

    switch(address) {
        case 0x4016:
            controller = CONTROLLER_1;
            break;
        case 0x4017:
            controller = CONTROLLER_2;
            break;
        default:
            break;
    }

    if(controller < CONTROLLER_MAX) {

        if(input->button[controller].position < BUTTON_MAX) {
            result.state = input->button[controller].state[input->button[controller].position++] ? 1 : 0;
        }
    }

    return result.raw;
}

nesl_error_e nesl_input_reset(nesl_input_t *input)
{

    for(nesl_controller_e controller = 0; controller < CONTROLLER_MAX; ++controller) {

        for(nesl_button_e button = 0; button < BUTTON_MAX; ++button) {
            input->button[controller].state[button] = false;
        }

        input->button[controller].position = BUTTON_MAX;
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
    nesl_strobe_t strobe = {};

    switch(address) {
        case 0x4016:
            strobe.raw = data;

            if(input->strobe) {

                if(!strobe.state) {

                    for(nesl_controller_e controller = 0; controller < CONTROLLER_MAX; ++controller) {

                        for(nesl_button_e button = 0; button < BUTTON_MAX; ++button) {
                            input->button[controller].state[button] = nesl_service_get_button(controller, button);
                        }

                        input->button[controller].position = BUTTON_A;
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
