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
    nesl_audio_t audio; /*< Audio context */

    struct {
        NESL_ServiceGetAudio callback;  /*< Audio callback */
        void *context;                  /*< Audio callback context */
    } setup;

    struct {

        struct {
            uint16_t address;   /*< Read/write address */
            uint8_t data;       /*< Read/write data */
            bool cycle;         /*< Cycle state */
            bool initialized;   /*< Initialized state */
            bool reset;         /*< Reset state */
        } square[NESL_CHANNEL_MAX];
    } synthesizer;
} nesl_test_t;

static nesl_test_t g_test = {}; /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_error_e NESL_AudioBufferInit(nesl_audio_buffer_t *buffer, int length)
{
    return NESL_SUCCESS;
}

int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, float *data, int length)
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

void NESL_AudioSquareCycle(nesl_audio_square_t *square)
{

    for(nesl_channel_e channel = 0; channel < NESL_CHANNEL_MAX; ++channel) {

        if(square == &g_test.audio.synthesizer.square[channel]) {
            g_test.synthesizer.square[channel].cycle = true;
            break;
        }
    }
}

nesl_error_e NESL_AudioSquareInit(nesl_audio_square_t *square)
{
    nesl_error_e result = NESL_FAILURE;

    for(nesl_channel_e channel = 0; channel < NESL_CHANNEL_MAX; ++channel) {

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

nesl_error_e NESL_AudioSquareReset(nesl_audio_square_t *square)
{
    nesl_error_e result = NESL_FAILURE;

    for(nesl_channel_e channel = 0; channel < NESL_CHANNEL_MAX; ++channel) {

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

    for(nesl_channel_e channel = 0; channel < NESL_CHANNEL_MAX; ++channel) {

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

    for(nesl_channel_e channel = 0; channel < NESL_CHANNEL_MAX; ++channel) {

        if(square == &g_test.audio.synthesizer.square[channel]) {
            g_test.synthesizer.square[channel].address = address;
            g_test.synthesizer.square[channel].data = data;
            break;
        }
    }
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

static nesl_error_e NESL_TestInit(void)
{
    memset(&g_test, 0, sizeof(g_test));

    return NESL_AudioInit(&g_test.audio);
}

static nesl_error_e NESL_TestAudioCycle(void)
{
    nesl_error_e result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit() == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    /* TODO: TEST AUDIO CYCLE */

exit:
    NESL_TEST_RESULT(result);

    return result;
}

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
            && g_test.synthesizer.square[NESL_CHANNEL_1].initialized
            && g_test.synthesizer.square[NESL_CHANNEL_2].initialized)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

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
            && g_test.synthesizer.square[NESL_CHANNEL_1].reset
            && g_test.synthesizer.square[NESL_CHANNEL_2].reset)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

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
            && !g_test.synthesizer.square[NESL_CHANNEL_1].initialized
            && !g_test.synthesizer.square[NESL_CHANNEL_2].initialized)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

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

                if(NESL_ASSERT((g_test.synthesizer.square[NESL_CHANNEL_1].address == address)
                        && (g_test.synthesizer.square[NESL_CHANNEL_1].data == data))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x4004 ... 0x4007:

                if(NESL_ASSERT((g_test.synthesizer.square[NESL_CHANNEL_2].address == (address - 4))
                        && (g_test.synthesizer.square[NESL_CHANNEL_2].data == data))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x4008 ... 0x400B:

                /* TODO: TEST WRITE TO TRIANGLE SYNTHESIZER */

                break;
            case 0x400C ... 0x400F:

                /* TODO: TEST WRITE TO NOISE SYNTHESIZER */

                break;
            case 0x4010 ... 0x4013:

                /* TODO: TEST WRITE TO DMC SYNTHESIZER */

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
