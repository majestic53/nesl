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
 * @file NESL_audio.c
 * @brief Audio subsystem.
 */

#include "../../include/system/NESL_audio.h"
#include "../../include/NESL_service.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Get audio data callback.
 * @param context Pointer to audio context
 * @param data Pointer to data array
 * @param length Length of data array in bytes
 */
static void NESL_AudioGetData(void *context, uint8_t *data, int length)
{
    memset(data, 0, length);
    NESL_AudioBufferRead(&((nesl_audio_t *)context)->buffer, (float *)data, length / sizeof(float));
}

/**
 * @brief Get audio status register.
 * @param audio Pointer to audio subsystem context
 * @return Audio status data
 */
static uint8_t NESL_AudioGetStatus(nesl_audio_t *audio)
{
    nesl_audio_status_t result = {};

    /* TODO: GET STATUS */

    return result.raw;
}

/**
 * @brief Set audio frame register.
 * @param audio Pointer to audio subsystem context
 * @param data Audio frame data
 */
static void NESL_AudioSetFrame(nesl_audio_t *audio, uint8_t data)
{
    audio->frame.raw = data;

    /* TODO: SET FRAME */
}

/**
 * @brief Set audio status register.
 * @param audio Pointer to audio subsystem context
 * @param data Audio status data
 */
static void NESL_AudioSetStatus(nesl_audio_t *audio, uint8_t data)
{
    audio->status.raw = data;

    /* TODO: SET STATUS */
}

void NESL_AudioCycle(nesl_audio_t *audio, uint64_t cycle)
{

    if(!(cycle % 6)) {
        NESL_AudioSquareCycle(&audio->synthesizer.square[NESL_SYNTHESIZER_SQUARE_1]);
        NESL_AudioSquareCycle(&audio->synthesizer.square[NESL_SYNTHESIZER_SQUARE_2]);
        NESL_AudioTriangleCycle(&audio->synthesizer.triangle);
        NESL_AudioNoiseCycle(&audio->synthesizer.noise);
        NESL_AudioDMCCycle(&audio->synthesizer.dmc);

        if((NESL_AudioSquareReadable(&audio->synthesizer.square[NESL_SYNTHESIZER_SQUARE_1]) >= 128)
                && (NESL_AudioSquareReadable(&audio->synthesizer.square[NESL_SYNTHESIZER_SQUARE_2]) >= 128)
                && (NESL_AudioTriangleReadable(&audio->synthesizer.triangle) >= 128)
                && (NESL_AudioNoiseReadable(&audio->synthesizer.noise) >= 128)
                && (NESL_AudioDMCReadable(&audio->synthesizer.dmc) >= 128)) {
            float data[NESL_SYNTHESIZER_MAX + 1][128] = {};

            NESL_AudioSquareRead(&audio->synthesizer.square[NESL_SYNTHESIZER_SQUARE_1], data[NESL_SYNTHESIZER_SQUARE_1], 128);
            NESL_AudioSquareRead(&audio->synthesizer.square[NESL_SYNTHESIZER_SQUARE_2], data[NESL_SYNTHESIZER_SQUARE_2], 128);
            NESL_AudioTriangleRead(&audio->synthesizer.triangle, data[NESL_SYNTHESIZER_TRIANGLE], 128);
            NESL_AudioNoiseRead(&audio->synthesizer.noise, data[NESL_SYNTHESIZER_NOISE], 128);
            NESL_AudioDMCRead(&audio->synthesizer.dmc, data[NESL_SYNTHESIZER_DMC], 128);

            /* TODO: MIX SAMPLES[NESL_SYNTHESIZER_SQUARE_1-NESL_SYNTHESIZER_DMC] INTO SAMPLES[NESL_SYNTHESIZER_MAX] */

            NESL_AudioBufferWrite(&audio->buffer, data[NESL_SYNTHESIZER_MAX], 128);
        }


    }
}

nesl_error_e NESL_AudioInit(nesl_audio_t *audio)
{
    nesl_error_e result;

    if((result = NESL_AudioBufferInit(&audio->buffer, 2048)) == NESL_FAILURE) {
        goto exit;
    }

    for(nesl_synthesizer_e channel = NESL_SYNTHESIZER_SQUARE_1; channel <= NESL_SYNTHESIZER_SQUARE_2; ++channel) {

        if((result = NESL_AudioSquareInit(&audio->synthesizer.square[channel])) == NESL_FAILURE) {
            goto exit;
        }
    }

    if((result = NESL_AudioTriangleInit(&audio->synthesizer.triangle)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_AudioNoiseInit(&audio->synthesizer.noise)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_AudioDMCInit(&audio->synthesizer.dmc)) == NESL_FAILURE) {
        goto exit;
    }

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

nesl_error_e NESL_AudioReset(nesl_audio_t *audio)
{
    nesl_error_e result;

    audio->frame.raw = 0;
    audio->status.raw = 0;

    for(nesl_synthesizer_e channel = NESL_SYNTHESIZER_SQUARE_1; channel <= NESL_SYNTHESIZER_SQUARE_2; ++channel) {

        if((result = NESL_AudioSquareReset(&audio->synthesizer.square[channel])) == NESL_FAILURE) {
            goto exit;
        }
    }

    if((result = NESL_AudioTriangleReset(&audio->synthesizer.triangle)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_AudioNoiseReset(&audio->synthesizer.noise)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_AudioDMCReset(&audio->synthesizer.dmc)) == NESL_FAILURE) {
        goto exit;
    }

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
    NESL_AudioDMCUninit(&audio->synthesizer.dmc);
    NESL_AudioNoiseUninit(&audio->synthesizer.noise);
    NESL_AudioTriangleUninit(&audio->synthesizer.triangle);

    for(nesl_synthesizer_e channel = NESL_SYNTHESIZER_SQUARE_1; channel <= NESL_SYNTHESIZER_SQUARE_2; ++channel) {
        NESL_AudioSquareUninit(&audio->synthesizer.square[channel]);
    }

    NESL_AudioBufferUninit(&audio->buffer);
    memset(audio, 0, sizeof(*audio));
}

void NESL_AudioWrite(nesl_audio_t *audio, uint16_t address, uint8_t data)
{
    nesl_synthesizer_e channel;

    switch(address) {
        case 0x4000 ... 0x4007:

            if((channel = (address > 0x4003) ? NESL_SYNTHESIZER_SQUARE_2 : NESL_SYNTHESIZER_SQUARE_1) == NESL_SYNTHESIZER_SQUARE_2) {
                address -= 4;
            }

            NESL_AudioSquareWrite(&audio->synthesizer.square[channel], address, data);
            break;
        case 0x4008 ... 0x400B:
            NESL_AudioTriangleWrite(&audio->synthesizer.triangle, address, data);
            break;
        case 0x400C ... 0x400F:
            NESL_AudioNoiseWrite(&audio->synthesizer.noise, address, data);
            break;
        case 0x4010 ... 0x4013:
            NESL_AudioDMCWrite(&audio->synthesizer.dmc, address, data);
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
