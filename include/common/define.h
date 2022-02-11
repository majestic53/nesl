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

#ifndef NESL_DEFINE_H_
#define NESL_DEFINE_H_

#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NESL_MAJOR 0
#define NESL_MINOR 1
#define NESL_PATCH 1

enum {
    NESL_BUS_PROCESSOR = 0,
    NESL_BUS_VIDEO,
    NESL_BUS_VIDEO_OAM,
};

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

enum {
    NESL_INTERRUPT_RESET = 0,
    NESL_INTERRUPT_NON_MASKABLE,
    NESL_INTERRUPT_MASKABLE,
    NESL_INTERRUPT_MAPPER,
};

#endif /* NESL_DEFINE_H_ */
