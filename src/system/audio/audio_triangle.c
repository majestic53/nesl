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
 * @file audio_triangle.c
 * @brief Audio triangle-wave synthesizer.
 */

#include <audio_triangle.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void nesl_audio_triangle_cycle(nesl_audio_triangle_t *triangle, uint64_t cycle)
{
    /* TODO: CYCLE SYNTHESIZER */
}

nesl_error_e nesl_audio_triangle_initialize(nesl_audio_triangle_t *triangle)
{
    int result;

    if((result = nesl_audio_buffer_initialize(&triangle->buffer, 256)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int nesl_audio_triangle_read(nesl_audio_triangle_t *triangle, int16_t *data, int length)
{
    return nesl_audio_buffer_read(&triangle->buffer, data, length);
}

int nesl_audio_triangle_readable(nesl_audio_triangle_t *triangle)
{
    return nesl_audio_buffer_readable(&triangle->buffer);
}

nesl_error_e nesl_audio_triangle_reset(nesl_audio_triangle_t *triangle)
{
    int result;

    if((result = nesl_audio_buffer_reset(&triangle->buffer)) == NESL_FAILURE) {
        goto exit;
    }

    memset(&triangle->state, 0, sizeof(triangle->state));

exit:
    return result;
}

void nesl_audio_triangle_uninitialize(nesl_audio_triangle_t *triangle)
{
    nesl_audio_buffer_uninitialize(&triangle->buffer);
    memset(triangle, 0, sizeof(*triangle));
}

void nesl_audio_triangle_write(nesl_audio_triangle_t *triangle, uint16_t address, uint8_t data)
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
