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
 * @file NESL_error.c
 * @brief Get/Set global error string.
 */

#include "../../include/NESL_common.h"

/**
 * @struct nesl_error_t
 * @brief Contains the error context.
 */
typedef struct {
    char buffer[256];               /*< Error string */
} nesl_error_t;

static nesl_error_t g_error = {};   /*< Error context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

const char *NESL_GetError(void)
{
    return g_error.buffer;
}

nesl_error_e NESL_SetError(const char *file, const char *function, int line, const char *format, ...)
{
    va_list arguments;

    va_start(arguments, format);
    vsnprintf(g_error.buffer, sizeof(g_error.buffer), format, arguments);
    snprintf(g_error.buffer + strlen(g_error.buffer), sizeof(g_error.buffer) - strlen(g_error.buffer), " (%s:%s@%i)", function, file, line);
    va_end(arguments);

    return NESL_FAILURE;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
