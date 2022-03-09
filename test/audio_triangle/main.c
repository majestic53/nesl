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
 * @brief Test application for audio triangle-wave synthesizer.
 */

#include "../../include/system/audio/NESL_audio_triangle.h"
#include "../include/NESL_common.h"

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_audio_triangle_t triangle; /*< Audio triangle-wave synthesizer context */
    float buffer[256];              /*< Audio buffer */
} nesl_test_t;

static nesl_test_t g_test = {};     /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Copy audio data into circular buffer.
 * @param buffer Constant pointer to audio buffer context
 * @param data Pointer to data array
 * @param length Maximum number of entries in data array
 */
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

/**
 * @brief Calculate the distance between the left and right offet, with wrap-around.
 * @param max Maximum length (used to wrap-around)
 * @param left Left offset
 * @param right Right offset
 * @return Distance between offsets
 */
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

/**
 * @brief Calculate the minimum between the left and right offset.
 * @param left Left offset
 * @param right Right offset
 * @return Minimum between offsets
 */
static int NESL_AudioBufferMinimum(int left, int right)
{
    return (left > right) ? right : left;
}

/**
 * @brief Determine if buffer is empty.
 * @return true if empty, false otherwise
 */
static bool NESL_AudioBufferEmpty(nesl_audio_buffer_t *buffer)
{
    return !buffer->full && (buffer->write == buffer->read);
}

nesl_error_e NESL_AudioBufferInit(nesl_audio_buffer_t *buffer, int length)
{
    nesl_error_e result = NESL_SUCCESS;

    if(length != 256) {
        result = NESL_FAILURE;
        goto exit;
    }

    buffer->data = g_test.buffer;
    buffer->length = length;

exit:
    return result;
}

int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, float *data, int length)
{
    int result = 0;

    if(!NESL_AudioBufferEmpty(buffer)) {

        if((result = NESL_AudioBufferMinimum(NESL_AudioBufferDistance(buffer->length, buffer->read, buffer->write), length)) > 0) {
            NESL_AudioBufferCopyOut(buffer, data, result);
        }
    }

    return result;
}

int NESL_AudioBufferReadable(nesl_audio_buffer_t *buffer)
{
    return NESL_AudioBufferDistance(buffer->length, buffer->read, buffer->write);
}

nesl_error_e NESL_AudioBufferReset(nesl_audio_buffer_t *buffer)
{
    buffer->read = 0;
    buffer->write = 0;

    return NESL_SUCCESS;
}

void NESL_AudioBufferUninit(nesl_audio_buffer_t *buffer)
{
    memset(g_test.buffer, 0, 256 * sizeof(float));
    memset(buffer, 0, sizeof(*buffer));
}

/**
 * @brief Uninitialize test context.
 */
static void NESL_TestUninit(void)
{
    NESL_AudioTriangleUninit(&g_test.triangle);
    memset(&g_test, 0, sizeof(g_test));
}

/**
 * @brief Initialize test context.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static int NESL_TestInit(void)
{
    NESL_TestUninit();

    return NESL_AudioTriangleInit(&g_test.triangle);
}

/**
 * @brief Test audio triangle synthesizer cycle.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioTriangleCycle(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio triangle synthesizer initialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioTriangleInit(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio triangle synthesizer read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioTriangleRead(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio triangle synthesizer readable.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioTriangleReadable(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio triangle synthesizer reset.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioTriangleReset(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio triangle synthesizer uninitialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioTriangleUninit(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio triangle synthesizer write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioTriangleWrite(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    NESL_TEST_RESULT(result);

    return result;
}

int main(void)
{
    static const NESL_Test TEST[] = {
        NESL_TestAudioTriangleCycle, NESL_TestAudioTriangleInit, NESL_TestAudioTriangleRead, NESL_TestAudioTriangleReadable,
        NESL_TestAudioTriangleReset, NESL_TestAudioTriangleUninit, NESL_TestAudioTriangleWrite,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < NESL_TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    NESL_TestUninit();

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */