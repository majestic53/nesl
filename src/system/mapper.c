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

#include "../../include/system/mapper/mapper_0.h"
#include "../../include/system/mapper/mapper_1.h"
#include "../../include/system/mapper/mapper_2.h"
#include "../../include/system/mapper/mapper_3.h"
#include "../../include/system/mapper/mapper_4.h"
#include "../../include/system/mapper/mapper_30.h"
#include "../../include/system/mapper/mapper_66.h"

typedef struct {
    int type;
    int (*initialize)(nesl_mapper_t *mapper);
    int (*interrupt)(nesl_mapper_t *mapper);
    uint8_t (*ram_read)(nesl_mapper_t *mapper, int type, uint16_t address);
    void (*ram_write)(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data);
    int (*reset)(nesl_mapper_t *mapper);
    uint8_t (*rom_read)(nesl_mapper_t *mapper, int type, uint16_t address);
    void (*rom_write)(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data);
    void (*uninitialize)(nesl_mapper_t *mapper);
} nesl_mapper_context_t;

static const nesl_mapper_context_t CONTEXT[] = {
    {
        NESL_MAPPER_0,
        nesl_mapper_0_initialize,
        NULL,
        nesl_mapper_0_ram_read,
        nesl_mapper_0_ram_write,
        NULL,
        nesl_mapper_0_rom_read,
        NULL,
        NULL,
    },
    {
        NESL_MAPPER_1,
        nesl_mapper_1_initialize,
        NULL,
        nesl_mapper_1_ram_read,
        nesl_mapper_1_ram_write,
        nesl_mapper_1_reset,
        nesl_mapper_1_rom_read,
        nesl_mapper_1_rom_write,
        nesl_mapper_1_uninitialize,
    },
    {
        NESL_MAPPER_2,
        nesl_mapper_2_initialize,
        NULL,
        nesl_mapper_2_ram_read,
        nesl_mapper_2_ram_write,
        nesl_mapper_2_reset,
        nesl_mapper_2_rom_read,
        nesl_mapper_2_rom_write,
        nesl_mapper_2_uninitialize,
    },
    {
        NESL_MAPPER_3,
        nesl_mapper_3_initialize,
        NULL,
        nesl_mapper_3_ram_read,
        nesl_mapper_3_ram_write,
        nesl_mapper_3_reset,
        nesl_mapper_3_rom_read,
        nesl_mapper_3_rom_write,
        nesl_mapper_3_uninitialize,
    },
    {
        NESL_MAPPER_4,
        nesl_mapper_4_initialize,
        nesl_mapper_4_interrupt,
        nesl_mapper_4_ram_read,
        nesl_mapper_4_ram_write,
        nesl_mapper_4_reset,
        nesl_mapper_4_rom_read,
        nesl_mapper_4_rom_write,
        nesl_mapper_4_uninitialize,
    },
    {
        NESL_MAPPER_30,
        nesl_mapper_30_initialize,
        NULL,
        nesl_mapper_30_ram_read,
        nesl_mapper_30_ram_write,
        nesl_mapper_30_reset,
        nesl_mapper_30_rom_read,
        nesl_mapper_30_rom_write,
        nesl_mapper_30_uninitialize,
    },
    {
        NESL_MAPPER_66,
        nesl_mapper_66_initialize,
        NULL,
        nesl_mapper_66_ram_read,
        nesl_mapper_66_ram_write,
        nesl_mapper_66_reset,
        nesl_mapper_66_rom_read,
        nesl_mapper_66_rom_write,
        nesl_mapper_66_uninitialize,
    },
    };

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static int nesl_mapper_context_initialize(nesl_mapper_t *mapper)
{
    const nesl_mapper_context_t *context = NULL;
    int count = sizeof(CONTEXT) / sizeof(*(CONTEXT)), index, result = NESL_FAILURE;

    for(index = 0; index < count; ++index) {
        context = &CONTEXT[index];

        if((context->type == mapper->type) && context->initialize) {

            if((result = context->initialize(mapper)) == NESL_FAILURE) {
                goto exit;
            }
            break;
        }
    }

    if(index == count) {
        result = NESL_ERROR_SET("Unsupported mapper type -- %u", mapper->type);
        goto exit;
    }

    mapper->interrupt = context->interrupt;
    mapper->ram_read = context->ram_read;
    mapper->ram_write = context->ram_write;
    mapper->reset = context->reset;
    mapper->rom_read = context->rom_read;
    mapper->rom_write = context->rom_write;

exit:
    return result;
}

static void nesl_mapper_context_uninitialize(nesl_mapper_t *mapper)
{

    for(int index = 0; index < (sizeof(CONTEXT) / sizeof(*(CONTEXT))); ++index) {
        const nesl_mapper_context_t *context = &CONTEXT[index];

        if((context->type == mapper->type) && context->uninitialize) {
            context->uninitialize(mapper);
            break;
        }
    }

    mapper->interrupt = NULL;
    mapper->ram_read = NULL;
    mapper->ram_write = NULL;
    mapper->reset = NULL;
    mapper->rom_read = NULL;
    mapper->rom_write = NULL;
}

int nesl_mapper_initialize(nesl_mapper_t *mapper, const void *data, int length)
{
    int result;

    if((result = nesl_cartridge_initialize(&mapper->cartridge, data, length)) == NESL_FAILURE) {
        goto exit;
    }

    mapper->mirror = mapper->cartridge.header->flag_6.mirror;
    mapper->type = (mapper->cartridge.header->flag_7.type_high << 4) | mapper->cartridge.header->flag_6.type_low;

    if((result = nesl_mapper_context_initialize(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int nesl_mapper_interrupt(nesl_mapper_t *mapper)
{
    int result = NESL_SUCCESS;

    if(mapper->interrupt) {

        if((result = mapper->interrupt(mapper)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    return result;
}

uint8_t nesl_mapper_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_RAM_CHARACTER:
        case NESL_BANK_RAM_PROGRAM:

            if(mapper->ram_read) {
                result = mapper->ram_read(mapper, type, address);
            }
            break;
        case NESL_BANK_ROM_CHARACTER:
        case NESL_BANK_ROM_PROGRAM:

            if(mapper->rom_read) {
                result = mapper->rom_read(mapper, type, address);
            }
            break;
        default:
            break;
    }

    return result;
}

int nesl_mapper_reset(nesl_mapper_t *mapper)
{
    int result = NESL_SUCCESS;

    if(mapper->reset) {

        if((result = mapper->reset(mapper)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    return result;
}

void nesl_mapper_uninitialize(nesl_mapper_t *mapper)
{
    nesl_mapper_context_uninitialize(mapper);
    nesl_cartridge_uninitialize(&mapper->cartridge);
    memset(mapper, 0, sizeof(*mapper));
}

void nesl_mapper_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_RAM_CHARACTER:
        case NESL_BANK_RAM_PROGRAM:

            if(mapper->ram_write) {
                mapper->ram_write(mapper, type, address, data);
            }
            break;
        case NESL_BANK_ROM_CHARACTER:
        case NESL_BANK_ROM_PROGRAM:

            if(mapper->rom_write) {
                mapper->rom_write(mapper, type, address, data);
            }
            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
