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

#ifndef NESL_MAPPER_4_H_
#define NESL_MAPPER_4_H_

#include "../NESL_mapper.h"

typedef union {

    struct {
        uint8_t mode : 1;
    };

    uint8_t raw;
} nesl_mapper_4_mirror_t;

typedef union {

    struct {
        uint8_t unused : 6;
        uint8_t ram_read_only : 1;
        uint8_t ram_enable : 1;
    };

    uint8_t raw;
} nesl_mapper_4_protect_t;

typedef union {

    struct {
        uint8_t bank : 3;
        uint8_t unused : 3;
        uint8_t program : 1;
        uint8_t character : 1;
    };

    uint8_t raw;
} nesl_mapper_4_select_t;

typedef struct {

    struct {
        uint8_t index[8];
        uint8_t data;
    } bank;

    struct {
        bool enable;
        uint8_t count;
        uint8_t latch;
    } interrupt;

    nesl_mapper_4_select_t select;
    nesl_mapper_4_mirror_t mirror;
    nesl_mapper_4_protect_t protect;
} nesl_mapper_4_context_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_Mapper4Init(nesl_mapper_t *mapper);
int NESL_Mapper4Interrupt(nesl_mapper_t *mapper);
uint8_t NESL_Mapper4RamRead(nesl_mapper_t *mapper, int type, uint16_t address);
void NESL_Mapper4RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data);
int NESL_Mapper4Reset(nesl_mapper_t *mapper);
uint8_t NESL_Mapper4RomRead(nesl_mapper_t *mapper, int type, uint16_t address);
void NESL_Mapper4RomWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data);
void NESL_Mapper4Uninit(nesl_mapper_t *mapper);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_4_H_ */
