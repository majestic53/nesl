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
 * @file main.c
 * @brief Test application for audio square-wave synthesizer.
 */

#include "../../include/system/audio/NESL_audio_square.h"
#include "../include/NESL_common.h"

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_audio_buffer_t buffer; /*< Audio square-wave synthesizer context */
} nesl_test_t;

//static nesl_test_t g_test = {}; /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_error_e NESL_AudioBufferInit(nesl_audio_buffer_t *buffer, int length)
{

    /* TODO */

    return NESL_SUCCESS;
}

int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, float *data, int length)
{

    /* TODO */

    return 0;
}

int NESL_AudioBufferReadable(nesl_audio_buffer_t *buffer)
{

    /* TODO */

    return 0;
}

nesl_error_e NESL_AudioBufferReset(nesl_audio_buffer_t *buffer)
{

    /* TODO */

    return NESL_SUCCESS;
}

void NESL_AudioBufferUninit(nesl_audio_buffer_t *buffer)
{
    /* TODO */
}

/* TODO */

int main(void)
{
    static const NESL_Test TEST[] = {

        /* TODO */

        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < NESL_TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
