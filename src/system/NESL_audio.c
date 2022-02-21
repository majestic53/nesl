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

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static uint8_t NESL_AudioGetStatus(nesl_audio_t *audio)
{
    nesl_audio_status_t result = {};

    /* TODO */

    return result.raw;
}

static void NESL_AudioSetDmc(nesl_audio_t *audio, uint16_t address, uint8_t data)
{
    audio->channel.dmc.byte[address] = data;

    /* TODO */
}

static void NESL_AudioSetFrame(nesl_audio_t *audio, uint8_t data)
{
    audio->frame.raw = data;

    /* TODO */
}

static void NESL_AudioSetNoise(nesl_audio_t *audio, uint16_t address, uint8_t data)
{
    audio->channel.noise.byte[address] = data;

    /* TODO */
}

static void NESL_AudioSetSquare(nesl_audio_t *audio, uint16_t address, uint8_t data)
{
    audio->channel.square[(address > 3) ? 1 : 0].byte[(address > 3) ? (address - 4) : address] = data;

    /* TODO */
}

static void NESL_AudioSetTriangle(nesl_audio_t *audio, uint16_t address, uint8_t data)
{
    audio->channel.triangle.byte[address] = data;

    /* TODO */
}

static void NESL_AudioSetStatus(nesl_audio_t *audio, uint8_t data)
{
    audio->status.raw = data;

    /* TODO */
}

int NESL_AudioInit(nesl_audio_t *audio)
{
    return NESL_AudioReset(audio);
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
    memset(audio, 0, sizeof(*audio));

    return NESL_SUCCESS;
}

void NESL_AudioUninit(nesl_audio_t *audio)
{
    memset(audio, 0, sizeof(*audio));
}

void NESL_AudioWrite(nesl_audio_t *audio, uint16_t address, uint8_t data)
{

    switch(address) {
        case 0x4000 ... 0x4007:
            NESL_AudioSetSquare(audio, address - 0x4000, data);
            break;
        case 0x4008 ... 0x400B:
            NESL_AudioSetTriangle(audio, address - 0x4008, data);
            break;
        case 0x400C ... 0x400F:
            NESL_AudioSetNoise(audio, address - 0x400C, data);
            break;
        case 0x4010 ... 0x4013:
            NESL_AudioSetDmc(audio, address - 0x4010, data);
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
