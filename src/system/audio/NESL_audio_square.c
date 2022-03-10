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
 * @file NESL_audio_square.c
 * @brief Audio square-wave synthesizer.
 */

#include "../../../include/system/audio/NESL_audio_square.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void NESL_AudioSquareCycle(nesl_audio_square_t *square, uint64_t cycle)
{
    /* TODO: CYCLE SYNTHESIZER */
}

nesl_error_e NESL_AudioSquareInit(nesl_audio_square_t *square)
{
    int result;

    if((result = NESL_AudioBufferInit(&square->buffer, 256)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int NESL_AudioSquareRead(nesl_audio_square_t *square, int16_t *data, int length)
{
    return NESL_AudioBufferRead(&square->buffer, data, length);
}

int NESL_AudioSquareReadable(nesl_audio_square_t *square)
{
    return NESL_AudioBufferReadable(&square->buffer);
}

nesl_error_e NESL_AudioSquareReset(nesl_audio_square_t *square)
{
    int result;

    if((result = NESL_AudioBufferReset(&square->buffer)) == NESL_FAILURE) {
        goto exit;
    }

    memset(&square->state, 0, sizeof(square->state));

exit:
    return result;
}

void NESL_AudioSquareUninit(nesl_audio_square_t *square)
{
    NESL_AudioBufferUninit(&square->buffer);
    memset(square, 0, sizeof(*square));
}

void NESL_AudioSquareWrite(nesl_audio_square_t *square, uint16_t address, uint8_t data)
{
    square->state.byte[address - 0x4000] = data;

    switch(address) {
        case 0x4000:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x4001:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x4002:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        case 0x4003:

            /* TODO: PERFORM ANY NEEDED STATE-CHANGE OPERATIONS */

            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
