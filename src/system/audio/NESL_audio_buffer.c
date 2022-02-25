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

#include "../../../include/system/audio/NESL_audio_buffer.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static bool NESL_AudioBufferEmpty(nesl_audio_buffer_t *buffer)
{
    return buffer->write == buffer->read;
}

static bool NESL_AudioBufferFull(nesl_audio_buffer_t *buffer)
{
    return ((buffer->write + 1) % (1024 + 1)) == buffer->read;
}

int NESL_AudioBufferInit(nesl_audio_buffer_t *buffer)
{
    return NESL_AudioBufferReset(buffer);
}

int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, uint8_t *data, int length)
{
    int result = 0;

    if(!NESL_AudioBufferEmpty(buffer)) {

        /* TODO: READ UP-TO THE MIN(AVAL BYTES | LENGTH) FROM DATA, ADVANCE READ POINTER, RETURN LENGTH READ */
        result = 0;
        /* --- */
    }

    return result;
}

int NESL_AudioBufferReset(nesl_audio_buffer_t *buffer)
{
    buffer->read = 0;
    buffer->write = 0;

    return NESL_SUCCESS;
}

void NESL_AudioBufferUninit(nesl_audio_buffer_t *buffer)
{
    memset(buffer, 0, sizeof(*buffer));
}

int NESL_AudioBufferWrite(nesl_audio_buffer_t *buffer, const uint8_t *data, int length)
{
    int result = 0;

    if(!NESL_AudioBufferFull(buffer)) {

        /* TODO: WRITE UP-TO THE MIN(AVAL BYTES | LENGTH) INTO DATA, ADVANCE WRITE POINTER, RETURN LENGTH WRITTEN */
        result = 0;
        /* --- */
    }

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
