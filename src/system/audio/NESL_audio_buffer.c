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

bool NESL_AudioBufferEmpty(nesl_audio_buffer_t *buffer)
{
    return buffer->write == buffer->read;
}

bool NESL_AudioBufferFull(nesl_audio_buffer_t *buffer)
{
    return ((buffer->write + 1) % buffer->count) == buffer->read;
}

int NESL_AudioBufferInit(nesl_audio_buffer_t *buffer, int count, int capacity)
{
    int result = NESL_SUCCESS;

    buffer->capacity = capacity;
    buffer->count = count;

    if(!(buffer->data = calloc(buffer->count, sizeof(uint8_t *)))) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int index = 0; index < buffer->count; ++index) {

        if(!(buffer->data[index] = calloc(buffer->capacity, sizeof(uint8_t)))) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if(!(buffer->length = calloc(buffer->count, sizeof(int)))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_AudioBufferReset(buffer);

exit:

    if(result == NESL_FAILURE) {
        NESL_AudioBufferUninit(buffer);
    }

    return result;
}

int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, uint8_t *data, int *length)
{
    int result = NESL_FAILURE;

    if(!NESL_AudioBufferEmpty(buffer) && (*length >= buffer->length[buffer->read])) {
        memcpy(data, buffer->data[buffer->read], buffer->length[buffer->read]);
        *length = buffer->length[buffer->read];
        buffer->read = (buffer->read + 1) % buffer->count;
        result = NESL_SUCCESS;
    }

    return result;
}

void NESL_AudioBufferReset(nesl_audio_buffer_t *buffer)
{
    buffer->read = 0;
    buffer->write = 0;
}

void NESL_AudioBufferUninit(nesl_audio_buffer_t *buffer)
{

    if(buffer->data) {

        for(int index = 0; index < buffer->count; ++index) {

            if(buffer->data[index]) {
                free(buffer->data[index]);
                buffer->data[index] = NULL;
            }
        }

        free(buffer->data);
        buffer->data = NULL;
    }

    if(buffer->length) {
        free(buffer->length);
        buffer->length = NULL;
    }

    memset(buffer, 0, sizeof(*buffer));
}

int NESL_AudioBufferWrite(nesl_audio_buffer_t *buffer, uint8_t *data, int length)
{
    int result = NESL_FAILURE;

    if(!NESL_AudioBufferFull(buffer) && (length <= buffer->capacity)) {
        memcpy(buffer->data[buffer->write], data, length);
        buffer->length[buffer->write] = length;
        buffer->write = (buffer->write + 1) % buffer->count;
        result = NESL_SUCCESS;
    }

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
