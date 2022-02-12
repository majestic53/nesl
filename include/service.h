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

#ifndef NESL_SERVICE_H_
#define NESL_SERVICE_H_

#include "./common.h"

enum {
    NESL_BUTTON_A = 0,
    NESL_BUTTON_B,
    NESL_BUTTON_SELECT,
    NESL_BUTTON_START,
    NESL_BUTTON_UP,
    NESL_BUTTON_DOWN,
    NESL_BUTTON_LEFT,
    NESL_BUTTON_RIGHT,
    NESL_BUTTON_MAX,
};

enum {
    NESL_CONTROLLER_1 = 0,
    NESL_CONTROLLER_2,
    NESL_CONTROLLER_MAX,
};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool nesl_service_button(int controller, int button);

int nesl_service_initialize(const char *title, int fullscreen, int linear, int scale);

void nesl_service_pixel(uint8_t color, bool red, bool green, bool blue, uint8_t x, uint8_t y);

int nesl_service_poll(void);

int nesl_service_reset(void);

int nesl_service_show(void);

void nesl_service_uninitialize(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_SERVICE_H_ */
