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
 * @brief Test application for audio dmc synthesizer.
 */

#include <audio_dmc.h>
#include <test.h>

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_audio_dmc_t dmc;   /*< Audio dmc synthesizer context */
    int16_t buffer[256];    /*< Audio buffer */
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
static void nesl_audio_buffer_copy_out(nesl_audio_buffer_t *buffer, int16_t *data, int length)
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

/**
 * @brief Calculate the minimum between the left and right offset.
 * @param left Left offset
 * @param right Right offset
 * @return Minimum between offsets
 */
static int nesl_audio_buffer_minimum(int left, int right)
{
    return (left > right) ? right : left;
}

/**
 * @brief Determine if buffer is empty.
 * @return true if empty, false otherwise
 */
static bool nesl_audio_buffer_empty(nesl_audio_buffer_t *buffer)
{
    return !buffer->full && (buffer->write == buffer->read);
}

nesl_error_e nesl_audio_buffer_initialize(nesl_audio_buffer_t *buffer, int length)
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

int nesl_audio_buffer_read(nesl_audio_buffer_t *buffer, int16_t *data, int length)
{
    int result = 0;

    if(!nesl_audio_buffer_empty(buffer)) {

        if((result = nesl_audio_buffer_minimum(nesl_audio_buffer_distance(buffer->length, buffer->read, buffer->write), length)) > 0) {
            nesl_audio_buffer_copy_out(buffer, data, result);
        }
    }

    return result;
}

int nesl_audio_buffer_readable(nesl_audio_buffer_t *buffer)
{
    return nesl_audio_buffer_distance(buffer->length, buffer->read, buffer->write);
}

nesl_error_e nesl_audio_buffer_reset(nesl_audio_buffer_t *buffer)
{
    buffer->read = 0;
    buffer->write = 0;

    return NESL_SUCCESS;
}

void nesl_audio_buffer_uninitialize(nesl_audio_buffer_t *buffer)
{
    memset(g_test.buffer, 0, 256 * sizeof(*g_test.buffer));
    memset(buffer, 0, sizeof(*buffer));
}

/**
 * @brief Uninitialize test context.
 */
static void nesl_test_uninitialize(void)
{
    nesl_audio_dmc_uninitialize(&g_test.dmc);
    memset(&g_test, 0, sizeof(g_test));
}

/**
 * @brief Initialize test context.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static int nesl_test_initialize(void)
{
    nesl_test_uninitialize();

    return nesl_audio_dmc_initialize(&g_test.dmc);
}

/**
 * @brief Test audio DMC synthesizer cycle.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_dmc_cycle(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio DMC synthesizer initialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_dmc_initialize(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio DMC synthesizer read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_dmc_read(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio DMC synthesizer readable.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_dmc_readable(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio DMC synthesizer reset.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_dmc_reset(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio DMC synthesizer uninitialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_dmc_uninitialize(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio DMC synthesizer write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_dmc_write(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize()) == NESL_FAILURE) {
        goto exit;
    }

    /* TODO */

exit:
    TEST_RESULT(result);

    return result;
}

int main(void)
{
    static const test TEST[] = {
        nesl_test_audio_dmc_cycle, nesl_test_audio_dmc_initialize, nesl_test_audio_dmc_read, nesl_test_audio_dmc_readable,
        nesl_test_audio_dmc_reset, nesl_test_audio_dmc_uninitialize, nesl_test_audio_dmc_write,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    nesl_test_uninitialize();

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
