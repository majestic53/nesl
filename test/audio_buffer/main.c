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
 * @brief Test application for audio circular-buffer.
 */

#include "../../include/system/audio/NESL_audio_buffer.h"
#include "../include/NESL_common.h"

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_audio_buffer_t buffer; /*< Audio buffer context */
} nesl_test_t;

static nesl_test_t g_test = {}; /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_error_e NESL_SetError(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

/**
 * @brief Calculate the distance between the left and right offet, with wrap-around.
 * @param max Maximum length (used to wrap-around)
 * @param left Left offset
 * @param right Right offset
 * @return Distance between offsets
 */
static int NESL_TestDistance(int max, int left, int right)
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
static int NESL_TestMinimum(int left, int right)
{
    return (left > right) ? right : left;
}

/**
 * @brief Uninitialize test context.
 */
static void NESL_TestUninit(void)
{
    NESL_AudioBufferUninit(&g_test.buffer);
    memset(&g_test, 0, sizeof(g_test));
}

/**
 * @brief Initialize test context.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestInit(int length)
{
    NESL_TestUninit();

    return NESL_AudioBufferInit(&g_test.buffer, length);
}

/**
 * @brief Test audio buffer initialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioBufferInit(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT((g_test.buffer.data != NULL)
            && (g_test.buffer.length == 5)
            && (g_test.buffer.read == 0)
            && (g_test.buffer.write == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio buffer read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioBufferRead(void)
{
    nesl_error_e result = NESL_SUCCESS;
    float buffer[10] = {}, data = 1024.f;

    if(NESL_ASSERT(NESL_TestInit(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT(NESL_AudioBufferRead(&g_test.buffer, buffer, sizeof(buffer) / sizeof(*buffer)) == 0)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int index = 0; index < g_test.buffer.length; ++index) {
        g_test.buffer.data[index] = data;
        data *= 2.f;
    }

    for(int length = 0; length <= (sizeof(buffer) / sizeof(*buffer)); ++length) {

        for(int write = 0; write < g_test.buffer.length; ++write) {

            for(int read = 0; read < g_test.buffer.length; ++read) {
                int copied;

                g_test.buffer.read = read;
                g_test.buffer.write = write;
                memset(buffer, 0, sizeof(buffer));
                copied = NESL_AudioBufferRead(&g_test.buffer, buffer, length);

                if(read != write) {
                    int distance = NESL_TestMinimum(NESL_TestDistance(g_test.buffer.length, read, write), length);

                    if(NESL_ASSERT((copied == distance)
                            && (g_test.buffer.read == ((read + distance) % g_test.buffer.length))
                            && (g_test.buffer.write == write)
                            && (g_test.buffer.full == false))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    for(int index = 0; index < distance; ++index) {

                        if(NESL_ASSERT(buffer[index] == g_test.buffer.data[(read + index) % g_test.buffer.length])) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    }
                } else {

                    if(NESL_ASSERT((copied == 0)
                            && (g_test.buffer.read == read)
                            && (g_test.buffer.write == write)
                            && (g_test.buffer.full == false))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    for(int index = 0; index < sizeof(buffer) / sizeof(*buffer); ++index) {

                        if(NESL_ASSERT(buffer[index] == 0)) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    }
                }
            }
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio buffer readable.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioBufferReadable(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int write = 0; write < g_test.buffer.length; ++write) {

        for(int read = 0; read < g_test.buffer.length; ++read) {
            int length;

            g_test.buffer.read = read;
            g_test.buffer.write = write;
            length = NESL_AudioBufferReadable(&g_test.buffer);

            if(read != write) {

                if(NESL_ASSERT(length == NESL_TestDistance(g_test.buffer.length, read, write))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
            } else if(NESL_ASSERT(length == 0)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio buffer reset.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioBufferReset(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit(30) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    g_test.buffer.read = 10;
    g_test.buffer.write = 20;

    if(NESL_ASSERT((NESL_AudioBufferReset(&g_test.buffer) == NESL_SUCCESS)
            && (g_test.buffer.read == 0)
            && (g_test.buffer.write == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio buffer uninitialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioBufferUninit(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_AudioBufferUninit(&g_test.buffer);

    if(NESL_ASSERT((g_test.buffer.data == NULL)
            && (g_test.buffer.length == 0)
            && (g_test.buffer.read == 0)
            && (g_test.buffer.write == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio buffer write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioBufferWrite(void)
{
    nesl_error_e result = NESL_SUCCESS;
    float buffer[10] = {}, data = 1024.f;

    if(NESL_ASSERT(NESL_TestInit(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int index = 0; index < g_test.buffer.length; ++index) {
        buffer[index] = data;
        data *= 2.f;
    }

    for(int length = 0; length <= (sizeof(buffer) / sizeof(*buffer)); ++length) {

        for(int write = 0; write < g_test.buffer.length; ++write) {

            for(int read = 0; read < g_test.buffer.length; ++read) {

                if(length > 0) {
                    int copied, distance;

                    g_test.buffer.read = read;
                    g_test.buffer.write = write;
                    g_test.buffer.full = false;
                    memset(g_test.buffer.data, 0, g_test.buffer.length * sizeof(*buffer));
                    copied = NESL_AudioBufferWrite(&g_test.buffer, buffer, length);
                    distance = NESL_TestMinimum(NESL_TestDistance(g_test.buffer.length, write, read), length);

                    if(NESL_ASSERT((copied == distance)
                            && (g_test.buffer.read == read)
                            && (g_test.buffer.write == ((write + distance) % g_test.buffer.length))
                            && (g_test.buffer.full == (copied && (g_test.buffer.write == g_test.buffer.read))))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    for(int index = 0; index < distance; ++index) {

                        if(NESL_ASSERT(buffer[index] == g_test.buffer.data[(write + index) % g_test.buffer.length])) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    }

                    if(read == write) {
                        g_test.buffer.read = read;
                        g_test.buffer.write = write;
                        g_test.buffer.full = true;
                        memset(g_test.buffer.data, 0, g_test.buffer.length * sizeof(*buffer));
                        copied = NESL_AudioBufferWrite(&g_test.buffer, buffer, length);

                        if(NESL_ASSERT((copied == 0)
                                && (g_test.buffer.read == read)
                                && (g_test.buffer.write == write)
                                && (g_test.buffer.full == true))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }

                        for(int index = 0; index < g_test.buffer.length; ++index) {

                            if(NESL_ASSERT(g_test.buffer.data[index] == 0)) {
                                result = NESL_FAILURE;
                                goto exit;
                            }
                        }
                    }

                }
            }
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio buffer writable.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioBufferWritable(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int write = 0; write < g_test.buffer.length; ++write) {

        for(int read = 0; read < g_test.buffer.length; ++read) {
            int length;

            g_test.buffer.read = read;
            g_test.buffer.write = write;
            length = NESL_AudioBufferWritable(&g_test.buffer);

            if(read != write) {

                if(NESL_ASSERT(length == NESL_TestDistance(g_test.buffer.length, write, read))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
            } else if(NESL_ASSERT(length == 0)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

int main(void)
{
    static const NESL_Test TEST[] = {
        NESL_TestAudioBufferInit, NESL_TestAudioBufferRead, NESL_TestAudioBufferReadable, NESL_TestAudioBufferReset,
        NESL_TestAudioBufferUninit, NESL_TestAudioBufferWrite, NESL_TestAudioBufferWritable,
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
