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

static void NESL_AudioBufferCopyIn(nesl_audio_buffer_t *buffer, const uint8_t *data, int length)
{

    if(buffer->write + length >= buffer->length) {
        memcpy(&buffer->data[buffer->write], data, buffer->length - length);
        data += buffer->length - length;
        length %= buffer->length;
        buffer->write = 0;
    }

    memcpy(&buffer->data[buffer->write], data, length);
    buffer->write += length;
}

static void NESL_AudioBufferCopyOut(nesl_audio_buffer_t *buffer, uint8_t *data, int length)
{

    if(buffer->read + length >= buffer->length) {
        memcpy(data, &buffer->data[buffer->read], buffer->length - length);
        data += buffer->length - length;
        length %= buffer->length;
        buffer->read = 0;
    }

    memcpy(data, &buffer->data[buffer->read], length);
    buffer->read += length;
}

static int NESL_AudioBufferDistance(int max, int left, int right)
{
    int result = 0;

    if(left < right) {
        result = right - left - 1;
    } else {
        result = max - (left - right + 1) - 1;
    }

    return result;
}

static int NESL_AudioBufferMinimum(int left, int right)
{
    return (left > right) ? right : left;
}

static bool NESL_AudioBufferEmpty(nesl_audio_buffer_t *buffer)
{
    return buffer->write == buffer->read;
}

static bool NESL_AudioBufferFull(nesl_audio_buffer_t *buffer)
{
    return ((buffer->write + 1) % buffer->length) == buffer->read;
}

int NESL_AudioBufferInit(nesl_audio_buffer_t *buffer, int length)
{
    int result = NESL_SUCCESS;

    if(!(buffer->data = calloc(++length, sizeof(uint8_t)))) {
        result = NESL_SET_ERROR("Failed to allocate buffer -- %u KB (%i bytes)", length / 1024.f, length);
        goto exit;
    }

    buffer->length = length;

    if((result = NESL_AudioBufferReset(buffer)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, uint8_t *data, int length)
{
    int result = 0;

    if(!NESL_AudioBufferEmpty(buffer)) {
        result = NESL_AudioBufferMinimum(NESL_AudioBufferDistance(buffer->length, buffer->read, buffer->write), length);
        NESL_AudioBufferCopyOut(buffer, data, result);
    }

    return result;
}

int NESL_AudioBufferReset(nesl_audio_buffer_t *buffer)
{
    memset(buffer->data, 0, buffer->length);
    buffer->read = 0;
    buffer->write = 0;

    return NESL_SUCCESS;
}

void NESL_AudioBufferUninit(nesl_audio_buffer_t *buffer)
{

    if(buffer->data) {
        free(buffer->data);
        buffer->data = NULL;
    }

    memset(buffer, 0, sizeof(*buffer));
}

int NESL_AudioBufferWrite(nesl_audio_buffer_t *buffer, const uint8_t *data, int length)
{
    int result = 0;

    if(!NESL_AudioBufferFull(buffer)) {
        result = NESL_AudioBufferMinimum(NESL_AudioBufferDistance(buffer->length, buffer->write, buffer->read), length);
        NESL_AudioBufferCopyIn(buffer, data, result);
    }

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
