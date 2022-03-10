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
 * @file NESL_audio_noise.c
 * @brief Audio noise synthesizer.
 */

#include "../../../include/system/audio/NESL_audio_noise.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void NESL_AudioNoiseCycle(nesl_audio_noise_t *noise, uint64_t cycle)
{
    /* TODO: CYCLE SYNTHESIZER */
}

nesl_error_e NESL_AudioNoiseInit(nesl_audio_noise_t *noise)
{
    int result;

    if((result = NESL_AudioBufferInit(&noise->buffer, 256)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int NESL_AudioNoiseRead(nesl_audio_noise_t *noise, int16_t *data, int length)
{
    return NESL_AudioBufferRead(&noise->buffer, data, length);
}

int NESL_AudioNoiseReadable(nesl_audio_noise_t *noise)
{
    return NESL_AudioBufferReadable(&noise->buffer);
}

nesl_error_e NESL_AudioNoiseReset(nesl_audio_noise_t *noise)
{
    int result;

    if((result = NESL_AudioBufferReset(&noise->buffer)) == NESL_FAILURE) {
        goto exit;
    }

    memset(&noise->state, 0, sizeof(noise->state));

exit:
    return result;
}

void NESL_AudioNoiseUninit(nesl_audio_noise_t *noise)
{
    NESL_AudioBufferUninit(&noise->buffer);
    memset(noise, 0, sizeof(*noise));
}

void NESL_AudioNoiseWrite(nesl_audio_noise_t *noise, uint16_t address, uint8_t data)
{
    noise->state.byte[address - 0x400C] = data;

    switch(address) {
        case 0x400C:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x400D:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x400E:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x400F:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
