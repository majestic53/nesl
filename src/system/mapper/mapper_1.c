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

#include "../../../include/system/mapper/mapper_1.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void nesl_mapper_1_set_character(nesl_mapper_t *mapper)
{
    const nesl_mapper_1_context_t *context = mapper->context;

    switch(context->control.character) {
        case 0:
            mapper->rom.character[0] = (context->character[0].bank >> 1) * 8 * 1024;
            mapper->rom.character[1] = mapper->rom.character[0] + (4 * 1024);
            break;
        case 1:
            mapper->rom.character[0] = context->character[0].bank * 4 * 1024;
            mapper->rom.character[1] = context->character[1].bank * 4 * 1024;
            break;
        default:
            break;
    }
}

static void nesl_mapper_1_set_mirror(nesl_mapper_t *mapper)
{

    switch(((nesl_mapper_1_context_t *)mapper->context)->control.mirror) {
        case 0:
            mapper->mirror = NESL_MIRROR_ONE_LOW;
            break;
        case 1:
            mapper->mirror = NESL_MIRROR_ONE_HIGH;
            break;
        case 2:
            mapper->mirror = NESL_MIRROR_VERTICAL;
            break;
        case 3:
            mapper->mirror = NESL_MIRROR_HORIZONTAL;
            break;
        default:
            break;
    }
}

static void nesl_mapper_1_set_program(nesl_mapper_t *mapper)
{
    const nesl_mapper_1_context_t *context = mapper->context;

    switch(context->control.program) {
        case 0 ... 1:
            mapper->rom.program[0] = (context->program.bank >> 1) * 32 * 1024;
            mapper->rom.program[1] = mapper->rom.program[0] + (16 * 1024);
            break;
        case 2:
            mapper->rom.program[0] = 0;
            mapper->rom.program[1] = context->program.bank * 16 * 1024;
            break;
        case 3:
            mapper->rom.program[0] = context->program.bank * 16 * 1024;
            mapper->rom.program[1] = (mapper->cartridge.header->rom.program - 1) * 16 * 1024;
            break;
        default:
            break;
    }
}

static void nesl_mapper_1_set(nesl_mapper_t *mapper, uint16_t address, uint8_t data)
{
    nesl_mapper_1_context_t *context = mapper->context;

    if(data & 0x80) {
        context->shift.data = 0;
        context->shift.position = 0;
    } else {
        context->shift.data = ((context->shift.data >> 1) | ((data & 1) << 4)) & 0x1F;

        if(++context->shift.position == 5) {

            switch(address) {
                case 0x8000 ... 0x9FFF:
                    context->control.raw = context->shift.data;
                    break;
                case 0xA000 ... 0xBFFF:
                    context->character[0].raw = context->shift.data;
                    break;
                case 0xC000 ... 0xDFFF:
                    context->character[1].raw = context->shift.data;
                    break;
                case 0xE000 ... 0xFFFF:
                    context->program.raw = context->shift.data;
                    break;
                default:
                    break;
            }

            nesl_mapper_1_set_mirror(mapper);
            nesl_mapper_1_set_character(mapper);
            nesl_mapper_1_set_program(mapper);
            context->shift.data = 0;
            context->shift.position = 0;
        }
    }
}

int nesl_mapper_1_initialize(nesl_mapper_t *mapper)
{
    int result = NESL_SUCCESS;

    if(!(mapper->context = calloc(1, sizeof(nesl_mapper_1_context_t)))) {
        result = NESL_ERROR_SET("Failed to allocate buffer -- %u KB (%i bytes)", sizeof(nesl_mapper_1_context_t), sizeof(nesl_mapper_1_context_t));
        goto exit;
    }

    if((result = nesl_mapper_1_reset(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

uint8_t nesl_mapper_1_ram_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_RAM_PROGRAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:

                    if(!((nesl_mapper_1_context_t *)mapper->context)->program.ram_disable) {
                        result = nesl_cartridge_ram_read(&mapper->cartridge, NESL_BANK_RAM_PROGRAM, mapper->ram.program + (address & 0x1FFF));
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return result;
}

void nesl_mapper_1_ram_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_RAM_PROGRAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:

                    if(!((nesl_mapper_1_context_t *)mapper->context)->program.ram_disable) {
                        nesl_cartridge_ram_write(&mapper->cartridge, NESL_BANK_RAM_PROGRAM, mapper->ram.program + (address & 0x1FFF), data);
                    }
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

int nesl_mapper_1_reset(nesl_mapper_t *mapper)
{
    ((nesl_mapper_1_context_t *)mapper->context)->control.raw = 0x0C;
    nesl_mapper_1_set_mirror(mapper);
    nesl_mapper_1_set_character(mapper);
    nesl_mapper_1_set_program(mapper);

    return NESL_SUCCESS;
}

uint8_t nesl_mapper_1_rom_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_ROM_CHARACTER:

            switch(address) {
                case 0x0000 ... 0x0FFF:
                    result = nesl_cartridge_rom_read(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[0] + (address & 0x0FFF));
                    break;
                case 0x1000 ... 0x1FFF:
                    result = nesl_cartridge_rom_read(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[1] + (address & 0x0FFF));
                    break;
                default:
                    break;
            }
            break;
        case NESL_BANK_ROM_PROGRAM:

            switch(address) {
                case 0x8000 ... 0xBFFF:
                    result = nesl_cartridge_rom_read(&mapper->cartridge, NESL_BANK_ROM_PROGRAM, mapper->rom.program[0] + (address & 0x3FFF));
                    break;
                case 0xC000 ... 0xFFFF:
                    result = nesl_cartridge_rom_read(&mapper->cartridge, NESL_BANK_ROM_PROGRAM, mapper->rom.program[1] + (address & 0x3FFF));
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }

    return result;
}

void nesl_mapper_1_rom_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_ROM_CHARACTER:

            switch(address) {
                case 0x0000 ... 0x0FFF:
                    nesl_cartridge_ram_write(&mapper->cartridge, NESL_BANK_RAM_CHARACTER, mapper->rom.character[0] + (address & 0x0FFF), data);
                    break;
                case 0x1000 ... 0x1FFF:
                    nesl_cartridge_ram_write(&mapper->cartridge, NESL_BANK_RAM_CHARACTER, mapper->rom.character[1] + (address & 0x0FFF), data);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BANK_ROM_PROGRAM:

            switch(address) {
                case 0x8000 ... 0xFFFF:
                    nesl_mapper_1_set(mapper, address, data);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void nesl_mapper_1_uninitialize(nesl_mapper_t *mapper)
{

    if(mapper->context) {
        free(mapper->context);
        mapper->context = NULL;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
