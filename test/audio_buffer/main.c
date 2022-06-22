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
 * @file main.c
 * @brief Test application for audio circular-buffer.
 */

#include <audio_buffer.h>
#include <test.h>

/*!
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_audio_buffer_t buffer; /*!< Audio buffer context */
} nesl_test_t;

static nesl_test_t g_test = {}; /*!< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_error_e nesl_set_error(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

/*!
 * @brief Calculate the distance between the left and right offet, with wrap-around.
 * @param[in] count Maximum count (used to wrap-around)
 * @param[in] left Left offset
 * @param[in] right Right offset
 * @return Distance between offsets
 */
static int nesl_test_distance(int count, int left, int right)
{
    int result = 0;

    if(left <= right) {
        result = right - left;
    } else {
        result = (count - left) + right;
    }

    return result;
}

/*!
 * @brief Calculate the minimum between the left and right offset.
 * @param[in] left Left offset
 * @param[in] right Right offset
 * @return Minimum between offsets
 */
static int nesl_test_minimum(int left, int right)
{
    return (left > right) ? right : left;
}

/*!
 * @brief Uninitialize test context.
 */
static void nesl_test_uninitialize(void)
{
    nesl_audio_buffer_uninitialize(&g_test.buffer);
    memset(&g_test, 0, sizeof(g_test));
}

/*!
 * @brief Initialize test context.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_initialize(int count)
{
    nesl_test_uninitialize();

    return nesl_audio_buffer_initialize(&g_test.buffer, count);
}

/*!
 * @brief Test audio buffer initialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_buffer_initialize(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(ASSERT(nesl_test_initialize(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(ASSERT((g_test.buffer.data != NULL)
            && (g_test.buffer.count == 5)
            && (g_test.buffer.read == 0)
            && (g_test.buffer.write == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test audio buffer read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_buffer_read(void)
{
    nesl_error_e result = NESL_SUCCESS;
    float buffer[10] = {}, data = 1024.f;

    if(ASSERT(nesl_test_initialize(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(ASSERT(nesl_audio_buffer_read(&g_test.buffer, buffer, sizeof(buffer) / sizeof(*buffer)) == 0)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int index = 0; index < g_test.buffer.count; ++index) {
        g_test.buffer.data[index] = data;
        data *= 2.f;
    }

    for(int count = 0; count <= (sizeof(buffer) / sizeof(*buffer)); ++count) {

        for(int write = 0; write < g_test.buffer.count; ++write) {

            for(int read = 0; read < g_test.buffer.count; ++read) {
                int copied;

                g_test.buffer.read = read;
                g_test.buffer.write = write;
                memset(buffer, 0, sizeof(buffer));
                copied = nesl_audio_buffer_read(&g_test.buffer, buffer, count);

                if(read != write) {
                    int distance = nesl_test_minimum(nesl_test_distance(g_test.buffer.count, read, write), count);

                    if(ASSERT((copied == distance)
                            && (g_test.buffer.read == ((read + distance) % g_test.buffer.count))
                            && (g_test.buffer.write == write)
                            && (g_test.buffer.full == false))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    for(int index = 0; index < distance; ++index) {

                        if(ASSERT(buffer[index] == g_test.buffer.data[(read + index) % g_test.buffer.count])) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    }
                } else {

                    if(ASSERT((copied == 0)
                            && (g_test.buffer.read == read)
                            && (g_test.buffer.write == write)
                            && (g_test.buffer.full == false))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    for(int index = 0; index < sizeof(buffer) / sizeof(*buffer); ++index) {

                        if(ASSERT(buffer[index] == 0)) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    }
                }
            }
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test audio buffer read/write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_buffer_read_write(void)
{
    int index, read, write;
    nesl_error_e result = NESL_SUCCESS;
    float consumer[2] = {}, producer[20] = {};

    if(ASSERT(nesl_test_initialize(10) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    read = g_test.buffer.read;
    write = g_test.buffer.write;

    for(index = 0; index < 20; ++index) {
        producer[index] = (index + 1) * 1024.f;
    }

    for(int trial = 0; trial < 10; ++trial) {

        if(ASSERT((nesl_audio_buffer_write(&g_test.buffer, &producer[trial * 2], 2) == 2)
                && (g_test.buffer.read == read)
                && (g_test.buffer.write == ((write + 2) % g_test.buffer.count))
                && !g_test.buffer.full)) {
            result = NESL_FAILURE;
            goto exit;
        }

        read = g_test.buffer.read;
        write = g_test.buffer.write;
        memset(consumer, 0, sizeof(consumer));

        if(ASSERT((nesl_audio_buffer_read(&g_test.buffer, consumer, 2) == 2)
                && (g_test.buffer.read == ((read + 2) % g_test.buffer.count))
                && (g_test.buffer.write == write
                && !g_test.buffer.full))) {
            result = NESL_FAILURE;
            goto exit;
        }

        for(index = 0; index < 2; ++index) {

            if(ASSERT(consumer[index] == producer[(trial * 2) + index])) {
                result = NESL_FAILURE;
                goto exit;
            }
        }

        read = g_test.buffer.read;
        write = g_test.buffer.write;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test audio buffer readable.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_buffer_readable(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(ASSERT(nesl_test_initialize(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int write = 0; write < g_test.buffer.count; ++write) {

        for(int read = 0; read < g_test.buffer.count; ++read) {
            int count;

            g_test.buffer.read = read;
            g_test.buffer.write = write;
            count = nesl_audio_buffer_readable(&g_test.buffer);

            if(read != write) {

                if(ASSERT(count == nesl_test_distance(g_test.buffer.count, read, write))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
            } else if(ASSERT(count == 0)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test audio buffer reset.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_buffer_reset(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(ASSERT(nesl_test_initialize(30) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    g_test.buffer.read = 10;
    g_test.buffer.write = 20;

    if(ASSERT((nesl_audio_buffer_reset(&g_test.buffer) == NESL_SUCCESS)
            && (g_test.buffer.read == 0)
            && (g_test.buffer.write == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test audio buffer uninitialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_buffer_uninitialize(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(ASSERT(nesl_test_initialize(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_audio_buffer_uninitialize(&g_test.buffer);

    if(ASSERT((g_test.buffer.data == NULL)
            && (g_test.buffer.count == 0)
            && (g_test.buffer.read == 0)
            && (g_test.buffer.write == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test audio buffer write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_buffer_write(void)
{
    nesl_error_e result = NESL_SUCCESS;
    float buffer[10] = {}, data = 1024.f;

    if(ASSERT(nesl_test_initialize(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int index = 0; index < g_test.buffer.count; ++index) {
        buffer[index] = data;
        data *= 2.f;
    }

    for(int count = 0; count <= (sizeof(buffer) / sizeof(*buffer)); ++count) {

        for(int write = 0; write < g_test.buffer.count; ++write) {

            for(int read = 0; read < g_test.buffer.count; ++read) {
                int copied, distance;

                g_test.buffer.read = read;
                g_test.buffer.write = write;
                g_test.buffer.full = false;
                memset(g_test.buffer.data, 0, g_test.buffer.count * sizeof(*buffer));
                copied = nesl_audio_buffer_write(&g_test.buffer, buffer, count);

                if(!(distance = nesl_test_distance(g_test.buffer.count, write, read))) {
                    distance = g_test.buffer.count;
                }

                distance = nesl_test_minimum(distance, count);

                if(ASSERT((copied == distance)
                        && (g_test.buffer.read == read)
                        && (g_test.buffer.write == ((write + distance) % g_test.buffer.count))
                        && (g_test.buffer.full == (copied && (g_test.buffer.write == g_test.buffer.read))))) {
                    result = NESL_FAILURE;
                    goto exit;
                }

                for(int index = 0; index < distance; ++index) {

                    if(ASSERT(buffer[index] == g_test.buffer.data[(write + index) % g_test.buffer.count])) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }

                if(read == write) {
                    g_test.buffer.read = read;
                    g_test.buffer.write = write;
                    g_test.buffer.full = true;
                    memset(g_test.buffer.data, 0, g_test.buffer.count * sizeof(*buffer));
                    copied = nesl_audio_buffer_write(&g_test.buffer, buffer, count);
                    distance = nesl_test_minimum(nesl_test_distance(g_test.buffer.count, write, read), count);

                    if(ASSERT((copied == distance)
                            && (g_test.buffer.read == read)
                            && (g_test.buffer.write == write)
                            && (g_test.buffer.full == true))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    for(int index = 0; index < g_test.buffer.count; ++index) {

                        if(ASSERT(g_test.buffer.data[index] == 0)) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    }
                }
            }
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/*!
 * @brief Test audio buffer writable.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_audio_buffer_writable(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(ASSERT(nesl_test_initialize(5) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(int write = 0; write < g_test.buffer.count; ++write) {

        for(int read = 0; read < g_test.buffer.count; ++read) {
            int count;

            g_test.buffer.read = read;
            g_test.buffer.write = write;
            g_test.buffer.full = false;
            count = nesl_audio_buffer_writable(&g_test.buffer);

            if(read != write) {

                if(ASSERT(count == nesl_test_distance(g_test.buffer.count, write, read))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
            } else if(ASSERT(count == g_test.buffer.count)) {
                result = NESL_FAILURE;
                goto exit;
            }

            g_test.buffer.full = true;
            count = nesl_audio_buffer_writable(&g_test.buffer);

            if(read != write) {

                if(ASSERT(count == nesl_test_distance(g_test.buffer.count, write, read))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
            } else if(ASSERT(count == 0)) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

int main(void)
{
    const test TEST[] = {
        nesl_test_audio_buffer_initialize, nesl_test_audio_buffer_read, nesl_test_audio_buffer_read_write, nesl_test_audio_buffer_readable,
        nesl_test_audio_buffer_reset, nesl_test_audio_buffer_uninitialize, nesl_test_audio_buffer_write, nesl_test_audio_buffer_writable,
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
