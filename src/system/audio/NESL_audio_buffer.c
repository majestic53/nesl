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

static void NESL_AudioBufferCopyIn(nesl_audio_buffer_t *buffer, float *data, int length)
{

    if((buffer->write + length) >= buffer->length) {
        int offset = buffer->length - buffer->write;

        memcpy(&buffer->data[buffer->write], data, offset * sizeof(float));
        length -= offset;
        data += offset;
        buffer->write = 0;
    }

    memcpy(&buffer->data[buffer->write], data, length * sizeof(float));
    buffer->write += length;
    buffer->full = (buffer->write == buffer->read);
}

static void NESL_AudioBufferCopyOut(nesl_audio_buffer_t *buffer, float *data, int length)
{

    if((buffer->read + length) >= buffer->length) {
        int offset = buffer->length - buffer->read;

        memcpy(data, &buffer->data[buffer->read], offset * sizeof(float));
        length -= offset;
        data += offset;
        buffer->read = 0;
    }

    memcpy(data, &buffer->data[buffer->read], length * sizeof(float));
    buffer->read += length;
    buffer->full = false;
}

static int NESL_AudioBufferDistance(int max, int left, int right)
{
    int result = 0;

    if(left <= right) {
        result = right - left;
    } else {
        result = (max - left) + right;
    }

    return result;
}

static int NESL_AudioBufferMinimum(int left, int right)
{
    return (left > right) ? right : left;
}

static bool NESL_AudioBufferEmpty(nesl_audio_buffer_t *buffer)
{
    return !buffer->full && (buffer->write == buffer->read);
}

static bool NESL_AudioBufferFull(nesl_audio_buffer_t *buffer)
{
    return buffer->full;
}

int NESL_AudioBufferInit(nesl_audio_buffer_t *buffer, int length)
{
    int result = NESL_SUCCESS;

    if(pthread_mutex_init(&buffer->lock, NULL) == -1) {
        result = NESL_SET_ERROR("Failed to initialize lock -- %i: %s", errno, strerror(errno));
        goto exit;
    }

    if(!(buffer->data = calloc(length, sizeof(float)))) {
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

int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, float *data, int length)
{
    int result = 0;

    pthread_mutex_lock(&buffer->lock);

    if(!NESL_AudioBufferEmpty(buffer)) {

        if((result = NESL_AudioBufferMinimum(NESL_AudioBufferDistance(buffer->length, buffer->read, buffer->write), length)) > 0) {
            NESL_AudioBufferCopyOut(buffer, data, result);
        }
    }

    pthread_mutex_unlock(&buffer->lock);

    return result;
}

int NESL_AudioBufferReadable(nesl_audio_buffer_t *buffer)
{
    int result;

    pthread_mutex_lock(&buffer->lock);
    result = NESL_AudioBufferDistance(buffer->length, buffer->read, buffer->write);
    pthread_mutex_unlock(&buffer->lock);

    return result;
}

int NESL_AudioBufferReset(nesl_audio_buffer_t *buffer)
{
    pthread_mutex_lock(&buffer->lock);
    memset(buffer->data, 0, buffer->length);
    buffer->read = 0;
    buffer->write = 0;
    buffer->full = false;
    pthread_mutex_unlock(&buffer->lock);

    return NESL_SUCCESS;
}

void NESL_AudioBufferUninit(nesl_audio_buffer_t *buffer)
{

    if(buffer->data) {
        free(buffer->data);
        buffer->data = NULL;
    }

    pthread_mutex_lock(&buffer->lock);
    pthread_mutex_unlock(&buffer->lock);
    pthread_mutex_destroy(&buffer->lock);
    memset(buffer, 0, sizeof(*buffer));
}

int NESL_AudioBufferWrite(nesl_audio_buffer_t *buffer, float *data, int length)
{
    int result = 0;

    pthread_mutex_lock(&buffer->lock);

    if(!NESL_AudioBufferFull(buffer)) {

        if((result = NESL_AudioBufferMinimum(NESL_AudioBufferDistance(buffer->length, buffer->write, buffer->read), length)) > 0) {
            NESL_AudioBufferCopyIn(buffer, data, result);
        }
    }

    pthread_mutex_unlock(&buffer->lock);

    return result;
}

int NESL_AudioBufferWritable(nesl_audio_buffer_t *buffer)
{
    int result;

    pthread_mutex_lock(&buffer->lock);
    result = NESL_AudioBufferDistance(buffer->length, buffer->write, buffer->read);
    pthread_mutex_unlock(&buffer->lock);

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
