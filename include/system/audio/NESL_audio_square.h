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

#ifndef NESL_AUDIO_SQUARE_H_
#define NESL_AUDIO_SQUARE_H_

#include "./NESL_audio_buffer.h"

typedef union {

    struct {
        uint8_t volume : 4;
        uint8_t volume_const : 1;
        uint8_t loop : 1;
        uint8_t duty : 2;
    };

    uint8_t raw;
} nesl_audio_square_envelope_t;

typedef union {

    struct {
        uint8_t period_high : 3;
        uint8_t counter : 5;
    };

    uint8_t raw;
} nesl_audio_square_length_t;

typedef union {

    struct {
        uint8_t shift : 3;
        uint8_t negative : 1;
        uint8_t period : 3;
        uint8_t enable : 1;
    };

    uint8_t raw;
} nesl_audio_square_sweep_t;

typedef struct {
    nesl_audio_buffer_t buffer;

    union {

        struct {
            nesl_audio_square_envelope_t envelope;
            nesl_audio_square_sweep_t sweep;
            uint8_t period_low;
            nesl_audio_square_length_t length;
        };

        uint8_t byte[4];
    } state;
} nesl_audio_square_t;

void NESL_AudioSquareCycle(nesl_audio_square_t *square);
int NESL_AudioSquareInit(nesl_audio_square_t *square);
int NESL_AudioSquareReset(nesl_audio_square_t *square);
int NESL_AudioSquareRead(nesl_audio_square_t *square, float *data, int length);
int NESL_AudioSquareReadable(nesl_audio_square_t *square);
void NESL_AudioSquareUninit(nesl_audio_square_t *square);
void NESL_AudioSquareWrite(nesl_audio_square_t *square, uint16_t address, uint8_t data);

#endif /* NESL_AUDIO_SQUARE_H_ */
