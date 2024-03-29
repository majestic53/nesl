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
 * @file nesl.c
 * @brief NESL interface.
 */

#include <bus.h>
#include <service.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_error_e nesl(const nesl_t *context)
{
    int result;

    if((result = nesl_service_initialize(context->title, context->linear, context->scale)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_bus_initialize(context->data, context->length)) == NESL_FAILURE) {
        goto exit;
    }

    while((result = nesl_service_poll()) == NESL_SUCCESS) {

        while(!nesl_bus_cycle());

        if((result = nesl_service_redraw()) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    nesl_bus_uninitialize();
    nesl_service_uninitialize();

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
