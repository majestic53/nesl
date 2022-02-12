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

#ifndef NESL_MAPPER_1_H_
#define NESL_MAPPER_1_H_

#include "../mapper.h"

typedef union {

    struct {
        uint8_t bank : 5;
    };

    uint8_t raw;
} nesl_mapper_1_character_t;

typedef union {

    struct {
        uint8_t mirror : 2;
        uint8_t program : 2;
        uint8_t character : 1;
    };

    uint8_t raw;
} nesl_mapper_1_control_t;

typedef union {

    struct {
        uint8_t bank : 4;
        uint8_t ram_disable : 1;
    };

    uint8_t raw;
} nesl_mapper_1_program_t;

typedef struct {

    struct {
        uint8_t data;
        uint8_t position;
    } shift;

    nesl_mapper_1_control_t control;
    nesl_mapper_1_character_t character[2];
    nesl_mapper_1_program_t program;
} nesl_mapper_1_context_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int nesl_mapper_1_initialize(nesl_mapper_t *mapper);

uint8_t nesl_mapper_1_ram_read(nesl_mapper_t *mapper, int type, uint16_t address);

void nesl_mapper_1_ram_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data);

uint8_t nesl_mapper_1_rom_read(nesl_mapper_t *mapper, int type, uint16_t address);

void nesl_mapper_1_rom_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data);

void nesl_mapper_1_uninitialize(nesl_mapper_t *mapper);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_1_H_ */
