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

#include "../../include/system/mapper/NESL_mapper_0.h"
#include "../../include/system/mapper/NESL_mapper_1.h"
#include "../../include/system/mapper/NESL_mapper_2.h"
#include "../../include/system/mapper/NESL_mapper_3.h"
#include "../../include/system/mapper/NESL_mapper_4.h"
#include "../../include/system/mapper/NESL_mapper_30.h"
#include "../../include/system/mapper/NESL_mapper_66.h"

typedef struct {
    int type;
    nesl_mapper_setup_t setup;
} nesl_mapper_context_t;

static const nesl_mapper_context_t CONTEXT[] = {
    {
        NESL_MAPPER_0,
        {
            NESL_Mapper0Init,
            NESL_Mapper0Uninit,
        },
    },
    {
        NESL_MAPPER_1,
        {
            NESL_Mapper1Init,
            NESL_Mapper1Uninit,
        },
    },
    {
        NESL_MAPPER_2,
        {
            NESL_Mapper2Init,
            NESL_Mapper2Uninit,
        },
    },
    {
        NESL_MAPPER_3,
        {
            NESL_Mapper3Init,
            NESL_Mapper3Uninit,
        },
    },
    {
        NESL_MAPPER_4,
        {
            NESL_Mapper4Init,
            NESL_Mapper4Uninit,
        },
    },
    {
        NESL_MAPPER_30,
        {
            NESL_Mapper30Init,
            NESL_Mapper30Uninit,
        },
    },
    {
        NESL_MAPPER_66,
        {
            NESL_Mapper66Init,
            NESL_Mapper66Uninit,
        },
    },
    };

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static int NESL_MapperContextInit(nesl_mapper_t *mapper)
{
    const nesl_mapper_context_t *context = NULL;
    int count = sizeof(CONTEXT) / sizeof(*(CONTEXT)), index, result = NESL_FAILURE;

    for(index = 0; index < count; ++index) {
        context = &CONTEXT[index];

        if(context->type == mapper->type) {

            if((result = context->setup.initialize(mapper)) == NESL_FAILURE) {
                goto exit;
            }
            break;
        }
    }

    if(index == count) {
        result = NESL_SET_ERROR("Unsupported mapper type -- %u", mapper->type);
        goto exit;
    }

exit:
    return result;
}

static void NESL_MapperContextUninit(nesl_mapper_t *mapper)
{

    for(int index = 0; index < (sizeof(CONTEXT) / sizeof(*(CONTEXT))); ++index) {
        const nesl_mapper_context_t *context = &CONTEXT[index];

        if(context->type == mapper->type) {
            context->setup.uninitialize(mapper);
            break;
        }
    }
}

int NESL_MapperInit(nesl_mapper_t *mapper, const void *data, int length)
{
    int result;

    if((result = NESL_CartridgeInit(&mapper->cartridge, data, length)) == NESL_FAILURE) {
        goto exit;
    }

    mapper->mirror = NESL_CartridgeGetMirror(&mapper->cartridge);
    mapper->type = NESL_CartridgeGetMapper(&mapper->cartridge);

    if((result = NESL_MapperContextInit(mapper)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_MapperReset(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int NESL_MapperInterrupt(nesl_mapper_t *mapper)
{
    return mapper->action.interrupt(mapper);
}

uint8_t NESL_MapperRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_RAM_CHARACTER:
        case NESL_BANK_RAM_PROGRAM:
            result = mapper->action.ram_read(mapper, type, address);
            break;
        case NESL_BANK_ROM_CHARACTER:
        case NESL_BANK_ROM_PROGRAM:
            result = mapper->action.rom_read(mapper, type, address);
            break;
        default:
            break;
    }

    return result;
}

int NESL_MapperReset(nesl_mapper_t *mapper)
{
    return mapper->action.reset(mapper);
}

void NESL_MapperUninit(nesl_mapper_t *mapper)
{
    NESL_MapperContextUninit(mapper);
    NESL_CartridgeUninit(&mapper->cartridge);
    memset(mapper, 0, sizeof(*mapper));
}

void NESL_MapperWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_RAM_CHARACTER:
        case NESL_BANK_RAM_PROGRAM:
            mapper->action.ram_write(mapper, type, address, data);
            break;
        case NESL_BANK_ROM_CHARACTER:
        case NESL_BANK_ROM_PROGRAM:
            mapper->action.rom_write(mapper, type, address, data);
            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
