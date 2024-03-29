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
 * @file define.h
 * @brief Common definitions.
 */

#ifndef NESL_DEFINE_H_
#define NESL_DEFINE_H_

#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define NESL_MAJOR 0            /*!< Major version */
#define NESL_MINOR 1            /*!< Minor version */
                                /*!< Patch version */
#define NESL_PATCH 0x531e019

/*!
 * @struct nesl_register_t
 * @brief 16-bit register type.
 */
typedef union {

    struct {
        uint8_t bit_0 : 1;      /*!< Bit-0 */
        uint8_t unused : 5;     /*!< Unused bit */
        uint8_t bit_6 : 1;      /*!< Bit-6 */
        uint8_t bit_7 : 1;      /*!< Bit-7 */
    };

    struct {
        uint8_t low;            /*!< Low byte */
        uint8_t high;           /*!< High byte */
    };

    uint16_t word;              /*!< Word */
} nesl_register_t;

#endif /* NESL_DEFINE_H_ */
