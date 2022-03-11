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

#include <audio_square.h>
#include <test.h>

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_audio_square_t square; /*< Audio square-wave synthesizer context */
    int16_t buffer[256];        /*< Audio buffer */
} nesl_test_t;

static nesl_test_t g_test = {}; /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Copy audio data into circular buffer.
 * @param buffer Constant pointer to audio buffer context
 * @param data Pointer to data array
 * @param length Maximum number of entries in data array
 */
static void NESL_AudioBufferCopyOut(nesl_audio_buffer_t *buffer, int16_t *data, int length)
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

nesl_error_e NESL_AudioBufferInitialize(nesl_audio_buffer_t *buffer, int length)
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

int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, int16_t *data, int length)
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

void NESL_AudioBufferUninitialize(nesl_audio_buffer_t *buffer)
{
    memset(g_test.buffer, 0, 256 * sizeof(*g_test.buffer));
    memset(buffer, 0, sizeof(*buffer));
}

/**
 * @brief Uninitialize test context.
 */
static void NESL_TestUninitialize(void)
{
    NESL_AudioSquareUninitialize(&g_test.square);
    memset(&g_test, 0, sizeof(g_test));
}

/**
 * @brief Initialize test context.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static int NESL_TestInitialize(void)
{
    NESL_TestUninitialize();

    return NESL_AudioSquareInitialize(&g_test.square);
}

/**
 * @brief Test audio square synthesizer cycle.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioSquareCycle(void)
{
    nesl_error_e result;

    if((result = NESL_TestInitialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio square synthesizer initialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioSquareInitialize(void)
{
    nesl_error_e result;

    if((result = NESL_TestInitialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio square synthesizer read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioSquareRead(void)
{
    nesl_error_e result;

    if((result = NESL_TestInitialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio square synthesizer readable.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioSquareReadable(void)
{
    nesl_error_e result;

    if((result = NESL_TestInitialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio square synthesizer reset.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioSquareReset(void)
{
    nesl_error_e result;

    if((result = NESL_TestInitialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio square synthesizer uninitialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioSquareUninitialize(void)
{
    nesl_error_e result;

    if((result = NESL_TestInitialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio square synthesizer write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioSquareWrite(void)
{
    nesl_error_e result;

    if((result = NESL_TestInitialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

int main(void)
{
    static const NESL_Test TEST[] = {
        NESL_TestAudioSquareCycle, NESL_TestAudioSquareInitialize, NESL_TestAudioSquareRead, NESL_TestAudioSquareReadable,
        NESL_TestAudioSquareReset, NESL_TestAudioSquareUninitialize, NESL_TestAudioSquareWrite,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    NESL_TestUninitialize();

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
