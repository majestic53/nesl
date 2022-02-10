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

#include "../../include/common.h"

typedef struct {
    char buffer[128];
    int error;
} nesl_err_t;

static nesl_err_t g_error = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

const char *nesl_error(void)
{
    return g_error.buffer;
}

int nesl_error_set(int error, const char *file, const char *function, int line, const char *format, ...)
{
    memset(&g_error, 0, sizeof(g_error));

    if(error != EXIT_SUCCESS) {
        va_list arguments;

        g_error.error = error;
        va_start(arguments, format);
        vsnprintf(g_error.buffer, sizeof(g_error.buffer), format, arguments);
        va_end(arguments);
        snprintf(g_error.buffer + strlen(g_error.buffer), sizeof(g_error.buffer) - strlen(g_error.buffer),
            " (%s:%s@%i)", function, file, line);
    }

    return g_error.error;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
