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

#ifndef NESL_MAPPER_0_H_
#define NESL_MAPPER_0_H_

#include "../mapper.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int nesl_mapper_0_initialize(nesl_mapper_t *mapper);

uint8_t nesl_mapper_0_ram_read(nesl_mapper_t *mapper, int type, uint16_t address);

void nesl_mapper_0_ram_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data);

uint8_t nesl_mapper_0_rom_read(nesl_mapper_t *mapper, int type, uint16_t address);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_MAPPER_0_H_ */
