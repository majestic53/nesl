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

#ifndef NESL_PROCESSOR_H_
#define NESL_PROCESSOR_H_

#include "../bus.h"

typedef union {

    struct {
        uint8_t carry : 1;
        uint8_t zero : 1;
        uint8_t interrupt_disable : 1;
        uint8_t decimal : 1;
        uint8_t breakpoint : 1;
        uint8_t unused : 1;
        uint8_t overflow : 1;
        uint8_t negative : 1;
    };

    uint8_t raw;
} nesl_processor_status_t;

typedef struct {
    uint8_t ram[2 * 1024];
    uint8_t cycle;

    union {

        struct {
            uint8_t transfer : 1;
            uint8_t transfer_sync : 1;
            uint8_t non_maskable : 1;
            uint8_t maskable : 1;
        };

        uint8_t raw;
    } event;

    struct {
        nesl_register_t accumulator;
        nesl_register_t program_counter;
        nesl_register_t stack_pointer;
        nesl_processor_status_t status;

        struct {
            nesl_register_t x;
            nesl_register_t y;
        } index;
    } state;

    struct {
        uint8_t data;
        nesl_register_t destination;
        nesl_register_t source;
    } transfer;
} nesl_processor_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void nesl_processor_cycle(nesl_processor_t *processor, uint64_t cycle);

int nesl_processor_initialize(nesl_processor_t *processor);

int nesl_processor_interrupt(nesl_processor_t *processor, bool maskable);

uint8_t nesl_processor_read(nesl_processor_t *processor, uint16_t address);

int nesl_processor_reset(nesl_processor_t *processor);

void nesl_processor_uninitialize(nesl_processor_t *processor);

void nesl_processor_write(nesl_processor_t *processor, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_PROCESSOR_H_ */
