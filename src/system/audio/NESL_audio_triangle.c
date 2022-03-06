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
 * @file NESL_audio_triangle.c
 * @brief Audio triangle-wave synthesizer.
 */

#include "../../../include/system/audio/NESL_audio_triangle.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void NESL_AudioTriangleCycle(nesl_audio_triangle_t *triangle)
{
    /* TODO: CYCLE SYNTHESIZER */
}

nesl_error_e NESL_AudioTriangleInit(nesl_audio_triangle_t *triangle)
{
    int result;

    if((result = NESL_AudioBufferInit(&triangle->buffer, 256)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int NESL_AudioTriangleRead(nesl_audio_triangle_t *triangle, float *data, int length)
{
    return NESL_AudioBufferRead(&triangle->buffer, data, length);
}

int NESL_AudioTriangleReadable(nesl_audio_triangle_t *triangle)
{
    return NESL_AudioBufferReadable(&triangle->buffer);
}

nesl_error_e NESL_AudioTriangleReset(nesl_audio_triangle_t *triangle)
{
    int result;

    if((result = NESL_AudioBufferReset(&triangle->buffer)) == NESL_FAILURE) {
        goto exit;
    }

    memset(&triangle->state, 0, sizeof(triangle->state));

exit:
    return result;
}

void NESL_AudioTriangleUninit(nesl_audio_triangle_t *triangle)
{
    NESL_AudioBufferUninit(&triangle->buffer);
    memset(triangle, 0, sizeof(*triangle));
}

void NESL_AudioTriangleWrite(nesl_audio_triangle_t *triangle, uint16_t address, uint8_t data)
{
    triangle->state.byte[address - 0x4008] = data;

    switch(address) {
        case 0x4008:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x4009:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x400A:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x400B:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
