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
 * @file error.h
 * @brief Get/Set global error string.
 */

#ifndef NESL_ERROR_H_
#define NESL_ERROR_H_

#include <define.h>

/*!
 * @brief Set global error macro.
 * @param[in] _FORMAT_ Error string format, followed by some number of arguments
 * @return NESL_FAILURE
 */
#define SET_ERROR(_FORMAT_, ...) \
    nesl_set_error(__FILE__, __FUNCTION__, __LINE__, _FORMAT_, __VA_ARGS__)

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Set global error.
 * @param[in] file Constant pointer to file string
 * @param[in] function Constant pointer to function string
 * @param[in] line File line
 * @param[in] format Error string format, followed by some number of arguments
 * @return NESL_FAILURE
 */
nesl_error_e nesl_set_error(const char *file, const char *function, int line, const char *format, ...);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_ERROR_H_ */
