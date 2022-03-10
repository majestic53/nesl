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
 * @brief Test application for audio subsystem.
 */

#include "../../include/system/NESL_audio.h"
#include "../../include/NESL_service.h"
#include "../include/NESL_common.h"

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_audio_t audio;                 /*< Audio context */

    struct {
        NESL_ServiceGetAudio callback;  /*< Audio callback */
        void *context;                  /*< Audio callback context */
    } setup;

    struct {

        struct {
            uint16_t address;           /*< Read/write address */
            uint8_t data;               /*< Read/write data */
            bool cycle;                 /*< Cycle state */
            bool initialized;           /*< Initialized state */
            bool reset;                 /*< Reset state */
        } square[NESL_SYNTHESIZER_SQUARE_2 + 1];

        struct {
            uint16_t address;           /*< Read/write address */
            uint8_t data;               /*< Read/write data */
            bool cycle;                 /*< Cycle state */
            bool initialized;           /*< Initialized state */
            bool reset;                 /*< Reset state */
        } triangle;

        struct {
            uint16_t address;           /*< Read/write address */
            uint8_t data;               /*< Read/write data */
            bool cycle;                 /*< Cycle state */
            bool initialized;           /*< Initialized state */
            bool reset;                 /*< Reset state */
        } noise;

        struct {
            uint16_t address;           /*< Read/write address */
            uint8_t data;               /*< Read/write data */
            bool cycle;                 /*< Cycle state */
            bool initialized;           /*< Initialized state */
            bool reset;                 /*< Reset state */
        } dmc;
    } synthesizer;
} nesl_test_t;

static nesl_test_t g_test = {};         /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_error_e NESL_AudioBufferInit(nesl_audio_buffer_t *buffer, int length)
{
    return NESL_SUCCESS;
}

int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, int16_t *data, int length)
{
    return 0;
}

nesl_error_e NESL_AudioBufferReset(nesl_audio_buffer_t *buffer)
{
    return NESL_SUCCESS;
}

void NESL_AudioBufferUninit(nesl_audio_buffer_t *buffer)
{
    return;
}

int NESL_AudioBufferWrite(nesl_audio_buffer_t *buffer, int16_t *data, int length)
{
    return 0;
}

void NESL_AudioDMCCycle(nesl_audio_dmc_t *dmc, uint64_t cycle)
{
    g_test.synthesizer.dmc.cycle = true;
}

nesl_error_e NESL_AudioDMCInit(nesl_audio_dmc_t *dmc)
{
    g_test.synthesizer.dmc.initialized = true;

    return NESL_SUCCESS;
}

int NESL_AudioDMCRead(nesl_audio_dmc_t *dmc, int16_t *data, int length)
{
    return 0;
}

int NESL_AudioDMCReadable(nesl_audio_dmc_t *dmc)
{
    return 0;
}

nesl_error_e NESL_AudioDMCReset(nesl_audio_dmc_t *dmc)
{
    g_test.synthesizer.dmc.reset = true;

    return NESL_SUCCESS;
}

void NESL_AudioDMCUninit(nesl_audio_dmc_t *dmc)
{
    g_test.synthesizer.dmc.initialized = false;
}

void NESL_AudioDMCWrite(nesl_audio_dmc_t *dmc, uint16_t address, uint8_t data)
{
    g_test.synthesizer.dmc.address = address;
    g_test.synthesizer.dmc.data = data;
}

void NESL_AudioNoiseCycle(nesl_audio_noise_t *noise, uint64_t cycle)
{
    g_test.synthesizer.noise.cycle = true;
}

nesl_error_e NESL_AudioNoiseInit(nesl_audio_noise_t *noise)
{
    g_test.synthesizer.noise.initialized = true;

    return NESL_SUCCESS;
}

int NESL_AudioNoiseRead(nesl_audio_noise_t *noise, int16_t *data, int length)
{
    return 0;
}

int NESL_AudioNoiseReadable(nesl_audio_noise_t *noise)
{
    return 0;
}

nesl_error_e NESL_AudioNoiseReset(nesl_audio_noise_t *noise)
{
    g_test.synthesizer.noise.reset = true;

    return NESL_SUCCESS;
}

void NESL_AudioNoiseUninit(nesl_audio_noise_t *noise)
{
    g_test.synthesizer.noise.initialized = false;
}

void NESL_AudioNoiseWrite(nesl_audio_noise_t *noise, uint16_t address, uint8_t data)
{
    g_test.synthesizer.noise.address = address;
    g_test.synthesizer.noise.data = data;
}

void NESL_AudioSquareCycle(nesl_audio_square_t *square, uint64_t cycle)
{

    for(nesl_synthesizer_e channel = NESL_SYNTHESIZER_SQUARE_1; channel <= NESL_SYNTHESIZER_SQUARE_2; ++channel) {

        if(square == &g_test.audio.synthesizer.square[channel]) {
            g_test.synthesizer.square[channel].cycle = true;
            break;
        }
    }
}

nesl_error_e NESL_AudioSquareInit(nesl_audio_square_t *square)
{
    nesl_error_e result = NESL_FAILURE;

    for(nesl_synthesizer_e channel = NESL_SYNTHESIZER_SQUARE_1; channel <= NESL_SYNTHESIZER_SQUARE_2; ++channel) {

        if(square == &g_test.audio.synthesizer.square[channel]) {

            if(!g_test.synthesizer.square[channel].initialized) {
                g_test.synthesizer.square[channel].initialized = true;
                result = NESL_SUCCESS;
            }
            break;
        }
    }

    return result;
}

int NESL_AudioSquareRead(nesl_audio_square_t *square, int16_t *data, int length)
{
    return 0;
}

int NESL_AudioSquareReadable(nesl_audio_square_t *square)
{
    return 0;
}

nesl_error_e NESL_AudioSquareReset(nesl_audio_square_t *square)
{
    nesl_error_e result = NESL_FAILURE;

    for(nesl_synthesizer_e channel = NESL_SYNTHESIZER_SQUARE_1; channel <= NESL_SYNTHESIZER_SQUARE_2; ++channel) {

        if(square == &g_test.audio.synthesizer.square[channel]) {
            g_test.synthesizer.square[channel].reset = true;
            result = NESL_SUCCESS;
            break;
        }
    }

    return result;
}

void NESL_AudioSquareUninit(nesl_audio_square_t *square)
{

    for(nesl_synthesizer_e channel = NESL_SYNTHESIZER_SQUARE_1; channel <= NESL_SYNTHESIZER_SQUARE_2; ++channel) {

        if(square == &g_test.audio.synthesizer.square[channel]) {

            if(g_test.synthesizer.square[channel].initialized) {
                g_test.synthesizer.square[channel].initialized = false;
            }
            break;
        }
    }
}

void NESL_AudioSquareWrite(nesl_audio_square_t *square, uint16_t address, uint8_t data)
{

    for(nesl_synthesizer_e channel = NESL_SYNTHESIZER_SQUARE_1; channel <= NESL_SYNTHESIZER_SQUARE_2; ++channel) {

        if(square == &g_test.audio.synthesizer.square[channel]) {
            g_test.synthesizer.square[channel].address = address;
            g_test.synthesizer.square[channel].data = data;
            break;
        }
    }
}

void NESL_AudioTriangleCycle(nesl_audio_triangle_t *triangle, uint64_t cycle)
{
    g_test.synthesizer.triangle.cycle = true;
}

nesl_error_e NESL_AudioTriangleInit(nesl_audio_triangle_t *triangle)
{
    g_test.synthesizer.triangle.initialized = true;

    return NESL_SUCCESS;
}

int NESL_AudioTriangleRead(nesl_audio_triangle_t *triangle, int16_t *data, int length)
{
    return 0;
}

int NESL_AudioTriangleReadable(nesl_audio_triangle_t *triangle)
{
    return 0;
}

nesl_error_e NESL_AudioTriangleReset(nesl_audio_triangle_t *triangle)
{
    g_test.synthesizer.triangle.reset = true;

    return NESL_SUCCESS;
}

void NESL_AudioTriangleUninit(nesl_audio_triangle_t *triangle)
{
    g_test.synthesizer.triangle.initialized = false;
}

void NESL_AudioTriangleWrite(nesl_audio_triangle_t *triangle, uint16_t address, uint8_t data)
{
    g_test.synthesizer.triangle.address = address;
    g_test.synthesizer.triangle.data = data;
}

nesl_error_e NESL_ServiceSetAudio(NESL_ServiceGetAudio callback, void *context)
{
    nesl_error_e result = NESL_SUCCESS;

    memset(&g_test.setup, 0, sizeof(g_test.setup));

    if(!callback || !context) {
        result = NESL_FAILURE;
        goto exit;
    }

    g_test.setup.callback = callback;
    g_test.setup.context = context;

exit:
    return result;
}

/**
 * @brief Initialize test context.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestInit(void)
{
    memset(&g_test, 0, sizeof(g_test));

    return NESL_AudioInit(&g_test.audio);
}

/**
 * @brief Test audio subsystem cycle.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioCycle(void)
{
    nesl_error_e result = NESL_SUCCESS;

    for(uint64_t cycle = 0; cycle <= 12; ++cycle) {
        bool expected = !(cycle % 6);

        if(NESL_ASSERT(NESL_TestInit() == NESL_SUCCESS)) {
            result = NESL_FAILURE;
            goto exit;
        }

        NESL_AudioCycle(&g_test.audio, cycle);

        if(NESL_ASSERT((g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_1].cycle == expected)
                && (g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_2].cycle == expected)
                && (g_test.synthesizer.triangle.cycle == expected)
                && (g_test.synthesizer.noise.cycle == expected)
                && (g_test.synthesizer.dmc.cycle == expected))) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio subsystem initialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioInit(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit() == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT((g_test.audio.status.raw == 0)
            && (g_test.audio.frame.raw == 0)
            && (g_test.setup.callback != NULL)
            && (g_test.setup.context == &g_test.audio)
            && g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_1].initialized
            && g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_2].initialized
            && g_test.synthesizer.triangle.initialized
            && g_test.synthesizer.noise.initialized
            && g_test.synthesizer.dmc.initialized)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio subsystem read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioRead(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit() == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(uint16_t address = 0x4000; address <= 0x4017; ++address) {

        switch(address) {
            case 0x4015:

                if(NESL_ASSERT(NESL_AudioRead(&g_test.audio, address) == 0)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            default:

                if(NESL_ASSERT(NESL_AudioRead(&g_test.audio, address) == 0)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio subsystem reset.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioReset(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit() == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT(NESL_AudioReset(&g_test.audio) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT((g_test.audio.status.raw == 0)
            && (g_test.audio.frame.raw == 0)
            && g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_1].reset
            && g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_2].reset
            && g_test.synthesizer.triangle.reset
            && g_test.synthesizer.noise.reset
            && g_test.synthesizer.dmc.reset)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio subsystem uninitialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioUninit(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit() == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_AudioUninit(&g_test.audio);

    if(NESL_ASSERT((g_test.audio.status.raw == 0)
            && (g_test.audio.frame.raw == 0)
            && !g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_1].initialized
            && !g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_2].initialized
            && !g_test.synthesizer.triangle.initialized
            && !g_test.synthesizer.noise.initialized
            && !g_test.synthesizer.dmc.initialized)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

/**
 * @brief Test audio subsystem write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e NESL_TestAudioWrite(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit() == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(uint16_t address = 0x4000, data = 0; address <= 0x4017; ++address, ++data) {
        NESL_AudioWrite(&g_test.audio, address, data);

        switch(address) {
            case 0x4000 ... 0x4003:

                if(NESL_ASSERT((g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_1].address == address)
                        && (g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_1].data == data))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x4004 ... 0x4007:

                if(NESL_ASSERT((g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_2].address == (address - 4))
                        && (g_test.synthesizer.square[NESL_SYNTHESIZER_SQUARE_2].data == data))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x4008 ... 0x400B:

                if(NESL_ASSERT((g_test.synthesizer.triangle.address == address)
                        && (g_test.synthesizer.triangle.data == data))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x400C ... 0x400F:

                if(NESL_ASSERT((g_test.synthesizer.noise.address == address)
                        && (g_test.synthesizer.noise.data == data))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x4010 ... 0x4013:

                if(NESL_ASSERT((g_test.synthesizer.dmc.address == address)
                        && (g_test.synthesizer.dmc.data == data))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x4015:

                if(NESL_ASSERT(g_test.audio.status.raw == data)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x4017:

                if(NESL_ASSERT(g_test.audio.frame.raw == data)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            default:
                break;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

int main(void)
{
    static const NESL_Test TEST[] = {
        NESL_TestAudioCycle, NESL_TestAudioInit, NESL_TestAudioRead, NESL_TestAudioReset,
        NESL_TestAudioUninit, NESL_TestAudioWrite,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < NESL_TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
