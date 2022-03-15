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
 * @file audio_buffer.c
 * @brief Audio circular-buffer.
 */

#include <audio_buffer.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Copy audio data into circular buffer.
 * @param[in,out] buffer Constant pointer to audio buffer context
 * @param[in] data Pointer to data array
 * @param[in] length Maximum number of entries in data array
 */
static void nesl_audio_buffer_copy_in(nesl_audio_buffer_t *buffer, float *data, int length)
{

    if((buffer->write + length) >= buffer->length) {
        int offset = buffer->length - buffer->write;

        memcpy(&buffer->data[buffer->write], data, offset * sizeof(*data));
        length -= offset;
        data += offset;
        buffer->write = 0;
    }

    memcpy(&buffer->data[buffer->write], data, length * sizeof(*data));
    buffer->write += length;
    buffer->full = (buffer->write == buffer->read);
}

/*!
 * @brief Copy audio data out of circular buffer.
 * @param[in,out] buffer Constant pointer to audio buffer context
 * @param[in] data Pointer to data array
 * @param[in] length Maximum number of entries in data array
 */
static void nesl_audio_buffer_copy_out(nesl_audio_buffer_t *buffer, float *data, int length)
{

    if((buffer->read + length) >= buffer->length) {
        int offset = buffer->length - buffer->read;

        memcpy(data, &buffer->data[buffer->read], offset * sizeof(*data));
        length -= offset;
        data += offset;
        buffer->read = 0;
    }

    memcpy(data, &buffer->data[buffer->read], length * sizeof(*data));
    buffer->read += length;
    buffer->full = false;
}

/*!
 * @brief Calculate the distance between the left and right offet, with wrap-around.
 * @param[in] max Maximum length (used to wrap-around)
 * @param[in] left Left offset
 * @param[in] right Right offset
 * @return Distance between offsets
 */
static int nesl_audio_buffer_distance(int max, int left, int right)
{
    int result = 0;

    if(left <= right) {
        result = right - left;
    } else {
        result = (max - left) + right;
    }

    return result;
}

/*!
 * @brief Calculate the minimum between the left and right offset.
 * @param[in] left Left offset
 * @param[in] right Right offset
 * @return Minimum between offsets
 */
static int nesl_audio_buffer_minimum(int left, int right)
{
    return (left > right) ? right : left;
}

/*!
 * @brief Determine if buffer is empty.
 * @return true if empty, false otherwise
 */
static bool nesl_audio_buffer_is_empty(nesl_audio_buffer_t *buffer)
{
    return !buffer->full && (buffer->write == buffer->read);
}

/*!
 * @brief Determine if buffer is full.
 * @return true if full, false otherwise
 */
static bool nesl_audio_buffer_is_full(nesl_audio_buffer_t *buffer)
{
    return buffer->full;
}

nesl_error_e nesl_audio_buffer_initialize(nesl_audio_buffer_t *buffer, int length)
{
    nesl_error_e result = NESL_SUCCESS;

    if(pthread_mutex_init(&buffer->lock, NULL) == -1) {
        result = SET_ERROR("Failed to initialize lock -- %i: %s", errno, strerror(errno));
        goto exit;
    }

    if(!(buffer->data = calloc(length, sizeof(*buffer->data)))) {
        result = SET_ERROR("Failed to allocate buffer -- %u KB (%i bytes)", length / 1024.f, length);
        goto exit;
    }

    buffer->length = length;

    if((result = nesl_audio_buffer_reset(buffer)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int nesl_audio_buffer_read(nesl_audio_buffer_t *buffer, float *data, int length)
{
    int result = 0;

    pthread_mutex_lock(&buffer->lock);

    if(!nesl_audio_buffer_is_empty(buffer)) {

        if((result = nesl_audio_buffer_minimum(nesl_audio_buffer_distance(buffer->length, buffer->read, buffer->write), length)) > 0) {
            nesl_audio_buffer_copy_out(buffer, data, result);
        }
    }

    pthread_mutex_unlock(&buffer->lock);

    return result;
}

int nesl_audio_buffer_readable(nesl_audio_buffer_t *buffer)
{
    int result;

    pthread_mutex_lock(&buffer->lock);
    result = nesl_audio_buffer_distance(buffer->length, buffer->read, buffer->write);
    pthread_mutex_unlock(&buffer->lock);

    return result;
}

nesl_error_e nesl_audio_buffer_reset(nesl_audio_buffer_t *buffer)
{
    pthread_mutex_lock(&buffer->lock);
    memset(buffer->data, 0, buffer->length);
    buffer->read = 0;
    buffer->write = 0;
    buffer->full = false;
    pthread_mutex_unlock(&buffer->lock);

    return NESL_SUCCESS;
}

void nesl_audio_buffer_uninitialize(nesl_audio_buffer_t *buffer)
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

int nesl_audio_buffer_write(nesl_audio_buffer_t *buffer, float *data, int length)
{
    int result = 0;

    pthread_mutex_lock(&buffer->lock);

    if(!nesl_audio_buffer_is_full(buffer)) {

        if(!(result = nesl_audio_buffer_distance(buffer->length, buffer->write, buffer->read))) {
            result = buffer->length;
        }

        if((result = nesl_audio_buffer_minimum(result, length)) > 0) {
            nesl_audio_buffer_copy_in(buffer, data, result);
        }
    }

    pthread_mutex_unlock(&buffer->lock);

    return result;
}

int nesl_audio_buffer_writable(nesl_audio_buffer_t *buffer)
{
    int result;

    pthread_mutex_lock(&buffer->lock);

    if(!(result = nesl_audio_buffer_distance(buffer->length, buffer->write, buffer->read))) {

        if(!nesl_audio_buffer_is_full(buffer)) {
            result = buffer->length;
        }
    }

    pthread_mutex_unlock(&buffer->lock);

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
