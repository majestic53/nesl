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
 * @file NESL.c
 * @brief Public API.
 */

#include <NESL_bus.h>
#include <NESL_service.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_Run(const nesl_t *context)
{
    int result;

    if((result = NESL_ServiceInitialize(context->title, context->fullscreen, context->linear, context->scale)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_BusInitialize(context->data, context->length)) == NESL_FAILURE) {
        goto exit;
    }

    while((result = NESL_ServicePoll()) == NESL_SUCCESS) {

        while(!NESL_BusCycle());

        if((result = NESL_ServiceRedraw()) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    NESL_BusUninitialize();
    NESL_ServiceUninitialize();

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
