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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void NESL_AudioGetData(void *context, uint8_t *data, int length)
{
    memset(data, 0, length);
    NESL_AudioBufferRead(&((nesl_audio_t *)context)->buffer, (float *)data, length / sizeof(float));
}

static uint8_t NESL_AudioGetStatus(nesl_audio_t *audio)
{
    nesl_audio_status_t result = {};

    /* TODO: GET STATUS */

    return result.raw;
}

static void NESL_AudioSetFrame(nesl_audio_t *audio, uint8_t data)
{
    audio->frame.raw = data;

    /* TODO: SET FRAME */
}

static void NESL_AudioSetStatus(nesl_audio_t *audio, uint8_t data)
{
    audio->status.raw = data;

    /* TODO: SET STATUS */
}

void NESL_AudioCycle(nesl_audio_t *audio, uint64_t cycle)
{

    if(!(cycle % 6)) {

        for(int channel = 0; channel < NESL_CHANNEL_MAX; ++channel) {
            NESL_AudioSquareCycle(&audio->synthesizer.square[channel]);
        }

        /* TODO: CYCLE SYNTHESIZERS */
        /* TODO: IF ENOUGH SAMPLES HAVE BEEN GENERATED, MIX TOGETHER AND ENQUEUE IN BUFFER */
    }
}

int NESL_AudioInit(nesl_audio_t *audio)
{
    int result;

    if((result = NESL_AudioBufferInit(&audio->buffer, 2048)) == NESL_FAILURE) {
        goto exit;
    }

    for(int channel = 0; channel < NESL_CHANNEL_MAX; ++channel) {

        if((result = NESL_AudioSquareInit(&audio->synthesizer.square[channel])) == NESL_FAILURE) {
            goto exit;
        }
    }

    /* TODO: INIT SYNTHESIZERS */

    if((result = NESL_AudioReset(audio)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

uint8_t NESL_AudioRead(nesl_audio_t *audio, uint16_t address)
{
    uint8_t result = 0;

    switch(address) {
        case 0x4015:
            result = NESL_AudioGetStatus(audio);
            break;
        default:
            break;
    }

    return result;
}

int NESL_AudioReset(nesl_audio_t *audio)
{
    int result;

    audio->frame.raw = 0;
    audio->status.raw = 0;

    for(int channel = 0; channel < NESL_CHANNEL_MAX; ++channel) {

        if((result = NESL_AudioSquareReset(&audio->synthesizer.square[channel])) == NESL_FAILURE) {
            goto exit;
        }
    }

    /* TODO: RESET SYNTHESIZERS */

    if((result = NESL_AudioBufferReset(&audio->buffer)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_ServiceSetAudio(NESL_AudioGetData, audio)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

void NESL_AudioUninit(nesl_audio_t *audio)
{

    /* TODO: UNINIT SYNTHESIZERS */

    for(int channel = 0; channel < NESL_CHANNEL_MAX; ++channel) {
        NESL_AudioSquareUninit(&audio->synthesizer.square[channel]);
    }

    NESL_AudioBufferUninit(&audio->buffer);
    memset(audio, 0, sizeof(*audio));
}

void NESL_AudioWrite(nesl_audio_t *audio, uint16_t address, uint8_t data)
{
    int channel;

    switch(address) {
        case 0x4000 ... 0x4007:

            if((channel = (address > 0x4003) ? NESL_CHANNEL_2 : NESL_CHANNEL_1) == NESL_CHANNEL_2) {
                address -= 4;
            }

            NESL_AudioSquareWrite(&audio->synthesizer.square[channel], address, data);
            break;
        case 0x4008 ... 0x400B:

            /* TODO: WRITE TO TRIANGLE SYNTHESIZER */

            break;
        case 0x400C ... 0x400F:

            /* TODO: WRITE TO NOISE SYNTHESIZER */

            break;
        case 0x4010 ... 0x4013:

            /* TODO: WRITE TO DMC SYNTHESIZER */

            break;
        case 0x4015:
            NESL_AudioSetStatus(audio, data);
            break;
        case 0x4017:
            NESL_AudioSetFrame(audio, data);
            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
