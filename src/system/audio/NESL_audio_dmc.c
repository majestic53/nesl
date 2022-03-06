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
 * @file NESL_audio_dmc.c
 * @brief Audio DMC synthesizer.
 */

#include "../../../include/system/audio/NESL_audio_dmc.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void NESL_AudioDMCCycle(nesl_audio_dmc_t *dmc)
{
    /* TODO: CYCLE SYNTHESIZER */
}

nesl_error_e NESL_AudioDMCInit(nesl_audio_dmc_t *dmc)
{
    int result;

    if((result = NESL_AudioBufferInit(&dmc->buffer, 256)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int NESL_AudioDMCRead(nesl_audio_dmc_t *dmc, float *data, int length)
{
    return NESL_AudioBufferRead(&dmc->buffer, data, length);
}

int NESL_AudioDMCReadable(nesl_audio_dmc_t *dmc)
{
    return NESL_AudioBufferReadable(&dmc->buffer);
}

nesl_error_e NESL_AudioDMCReset(nesl_audio_dmc_t *dmc)
{
    int result;

    if((result = NESL_AudioBufferReset(&dmc->buffer)) == NESL_FAILURE) {
        goto exit;
    }

    memset(&dmc->state, 0, sizeof(dmc->state));

exit:
    return result;
}

void NESL_AudioDMCUninit(nesl_audio_dmc_t *dmc)
{
    NESL_AudioBufferUninit(&dmc->buffer);
    memset(dmc, 0, sizeof(*dmc));
}

void NESL_AudioDMCWrite(nesl_audio_dmc_t *dmc, uint16_t address, uint8_t data)
{
    dmc->state.byte[address - 0x4010] = data;

    switch(address) {
        case 0x4010:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x4011:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x4012:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x4013:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
