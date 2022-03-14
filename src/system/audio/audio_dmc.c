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
 * @file audio_dmc.c
 * @brief Audio DMC synthesizer.
 */

#include <audio_dmc.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void nesl_audio_dmc_cycle(nesl_audio_dmc_t *dmc, float elapsed)
{
    /* TODO: CYCLE SYNTHESIZER */
}

nesl_error_e nesl_audio_dmc_initialize(nesl_audio_dmc_t *dmc)
{
    int result;

    if((result = nesl_audio_buffer_initialize(&dmc->buffer, 256)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int nesl_audio_dmc_read(nesl_audio_dmc_t *dmc, float *data, int length)
{
    return nesl_audio_buffer_read(&dmc->buffer, data, length);
}

int nesl_audio_dmc_readable(nesl_audio_dmc_t *dmc)
{
    return nesl_audio_buffer_readable(&dmc->buffer);
}

nesl_error_e nesl_audio_dmc_reset(nesl_audio_dmc_t *dmc)
{
    int result;

    if((result = nesl_audio_buffer_reset(&dmc->buffer)) == NESL_FAILURE) {
        goto exit;
    }

    memset(&dmc->state, 0, sizeof(dmc->state));

exit:
    return result;
}

void nesl_audio_dmc_uninitialize(nesl_audio_dmc_t *dmc)
{
    nesl_audio_buffer_uninitialize(&dmc->buffer);
    memset(dmc, 0, sizeof(*dmc));
}

void nesl_audio_dmc_write(nesl_audio_dmc_t *dmc, uint16_t address, uint8_t data)
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
