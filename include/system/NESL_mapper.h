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

#ifndef NESL_MAPPER_H_
#define NESL_MAPPER_H_

#include "../NESL_bus.h"
#include "./NESL_cartridge.h"

typedef struct nesl_mapper_s {
    nesl_cartridge_t cartridge;
    int mirror;
    int type;
    void *context;

    struct {
        uint32_t program;
    } ram;

    struct {
        uint32_t character[8];
        uint32_t program[4];
    } rom;

    int (*interrupt)(struct nesl_mapper_s *mapper);
    uint8_t (*ram_read)(struct nesl_mapper_s *mapper, int type, uint16_t address);
    void (*ram_write)(struct nesl_mapper_s *mapper, int type, uint16_t address, uint8_t data);
    int (*reset)(struct nesl_mapper_s *mapper);
    uint8_t (*rom_read)(struct nesl_mapper_s *mapper, int type, uint16_t address);
    void (*rom_write)(struct nesl_mapper_s *mapper, int type, uint16_t address, uint8_t data);
} nesl_mapper_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_MapperInit(nesl_mapper_t *mapper, const void *data, int length);
int NESL_MapperInterrupt(nesl_mapper_t *mapper);
uint8_t NESL_MapperRead(nesl_mapper_t *mapper, int type, uint16_t address);
int NESL_MapperReset(nesl_mapper_t *mapper);
void NESL_MapperUninit(nesl_mapper_t *mapper);
void NESL_MapperWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_H_ */
