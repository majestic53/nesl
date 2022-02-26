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

#include "../../include/system/NESL_audio.h"
#include "../../include/NESL_service.h"
#include "../include/NESL_common.h"

typedef struct {
    nesl_audio_t audio;

    struct {
        NESL_ServiceGetAudio callback;
        void *context;
    } setup;
} nesl_test_t;

static nesl_test_t g_test = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_AudioBufferInit(nesl_audio_buffer_t *buffer, int length)
{
    return NESL_SUCCESS;
}

int NESL_AudioBufferRead(nesl_audio_buffer_t *buffer, uint8_t *data, int length)
{
    return 0;
}

int NESL_AudioBufferReset(nesl_audio_buffer_t *buffer)
{
    return NESL_SUCCESS;
}

void NESL_AudioBufferUninit(nesl_audio_buffer_t *buffer)
{
    return;
}

int NESL_ServiceSetAudio(NESL_ServiceGetAudio callback, void *context)
{
    int result = NESL_SUCCESS;

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

static int NESL_TestInit(void)
{
    memset(&g_test, 0, sizeof(g_test));

    return NESL_AudioInit(&g_test.audio);
}

static int NESL_TestAudioInit(void)
{
    int result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit() == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT((g_test.audio.status.raw == 0)
            && (g_test.audio.frame.raw == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.square[0].byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.square[1].byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.triangle.byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.noise.byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.dmc.byte == 0)
            && (g_test.setup.callback != NULL)
            && (g_test.setup.context == &g_test.audio))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestAudioRead(void)
{
    int result = NESL_SUCCESS;

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

static int NESL_TestAudioReset(void)
{
    int result = NESL_SUCCESS;

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
            && (*(uint32_t *)g_test.audio.synthesizer.square[0].byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.square[1].byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.triangle.byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.noise.byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.dmc.byte == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestAudioUninit(void)
{
    int result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit() == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_AudioUninit(&g_test.audio);

    if(NESL_ASSERT((g_test.audio.status.raw == 0)
            && (g_test.audio.frame.raw == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.square[0].byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.square[1].byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.triangle.byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.noise.byte == 0)
            && (*(uint32_t *)g_test.audio.synthesizer.dmc.byte == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestAudioWrite(void)
{
    int result = NESL_SUCCESS;

    if(NESL_ASSERT(NESL_TestInit() == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    for(uint16_t address = 0x4000, data = 0; address <= 0x4017; ++address, ++data) {
        NESL_AudioWrite(&g_test.audio, address, data);

        switch(address) {
            case 0x4000 ... 0x4003:

                if(NESL_ASSERT(g_test.audio.synthesizer.square[0].byte[address - 0x4000] == data)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x4004 ... 0x4007:

                if(NESL_ASSERT(g_test.audio.synthesizer.square[1].byte[address - 0x4004] == data)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x4008 ... 0x400B:

                if(NESL_ASSERT(g_test.audio.synthesizer.triangle.byte[address - 0x4008] == data)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x400C ... 0x400F:

                if(NESL_ASSERT(g_test.audio.synthesizer.noise.byte[address - 0x400C] == data)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0x4010 ... 0x4013:

                if(NESL_ASSERT(g_test.audio.synthesizer.dmc.byte[address - 0x4010] == data)) {
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

static const NESL_Test TEST[] = {
    NESL_TestAudioInit,
    NESL_TestAudioRead,
    NESL_TestAudioReset,
    NESL_TestAudioUninit,
    NESL_TestAudioWrite,
    };

int main(void)
{
    int result = NESL_SUCCESS;

    for(int index = 0; index < NESL_TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
