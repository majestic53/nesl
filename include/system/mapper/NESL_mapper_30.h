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

#ifndef NESL_MAPPER_30_H_
#define NESL_MAPPER_30_H_

#include "../NESL_mapper.h"

typedef union {

    struct {
        uint8_t program : 5;
        uint8_t character : 2;
        uint8_t one_screen : 1;
    };

    uint8_t raw;
} nesl_mapper_30_bank_t;

typedef struct {
    nesl_mapper_30_bank_t bank;
} nesl_mapper_30_context_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_Mapper30Init(nesl_mapper_t *mapper);
int NESL_Mapper30Interrupt(nesl_mapper_t *mapper);
uint8_t NESL_Mapper30ReadRam(nesl_mapper_t *mapper, int type, uint16_t address);
uint8_t NESL_Mapper30ReadRom(nesl_mapper_t *mapper, int type, uint16_t address);
int NESL_Mapper30Reset(nesl_mapper_t *mapper);
void NESL_Mapper30WriteRam(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data);
void NESL_Mapper30WriteRom(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data);
void NESL_Mapper30Uninit(nesl_mapper_t *mapper);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_30_H_ */
