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

#include "../../../include/system/mapper/NESL_mapper_1.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void NESL_Mapper1SetCharacter(nesl_mapper_t *mapper)
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

static void NESL_Mapper1SetMirror(nesl_mapper_t *mapper)
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

static void NESL_Mapper1SetProgram(nesl_mapper_t *mapper)
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
            mapper->rom.program[1] = (NESL_CartridgeGetBankCount(&mapper->cartridge, NESL_BANK_ROM_PROGRAM) - 1) * 16 * 1024;
            break;
        default:
            break;
    }
}

static void NESL_Mapper1Set(nesl_mapper_t *mapper, uint16_t address, uint8_t data)
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

            NESL_Mapper1SetMirror(mapper);
            NESL_Mapper1SetCharacter(mapper);
            NESL_Mapper1SetProgram(mapper);
            context->shift.data = 0;
            context->shift.position = 0;
        }
    }
}

int NESL_Mapper1Init(nesl_mapper_t *mapper)
{
    int result = NESL_SUCCESS;

    if(!(mapper->context = calloc(1, sizeof(nesl_mapper_1_context_t)))) {
        result = NESL_SET_ERROR("Failed to allocate buffer -- %u KB (%i bytes)", sizeof(nesl_mapper_1_context_t), sizeof(nesl_mapper_1_context_t));
        goto exit;
    }

    mapper->action.interrupt = &NESL_Mapper1Interrupt;
    mapper->action.ram_read = &NESL_Mapper1RamRead;
    mapper->action.ram_write = &NESL_Mapper1RamWrite;
    mapper->action.reset = &NESL_Mapper1Reset;
    mapper->action.rom_read = &NESL_Mapper1RomRead;
    mapper->action.rom_write = &NESL_Mapper1RomWrite;

    if((result = NESL_Mapper1Reset(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int NESL_Mapper1Interrupt(nesl_mapper_t *mapper)
{
    return NESL_SUCCESS;
}

uint8_t NESL_Mapper1RamRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_RAM_PROGRAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:

                    if(!((nesl_mapper_1_context_t *)mapper->context)->program.ram_disable) {
                        result = NESL_CartridgeRamRead(&mapper->cartridge, NESL_BANK_RAM_PROGRAM, mapper->ram.program + (address & 0x1FFF));
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

void NESL_Mapper1RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_RAM_PROGRAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:

                    if(!((nesl_mapper_1_context_t *)mapper->context)->program.ram_disable) {
                        NESL_CartridgeRamWrite(&mapper->cartridge, NESL_BANK_RAM_PROGRAM, mapper->ram.program + (address & 0x1FFF), data);
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

int NESL_Mapper1Reset(nesl_mapper_t *mapper)
{
    ((nesl_mapper_1_context_t *)mapper->context)->control.raw = 0x0C;
    NESL_Mapper1SetMirror(mapper);
    NESL_Mapper1SetCharacter(mapper);
    NESL_Mapper1SetProgram(mapper);

    return NESL_SUCCESS;
}

uint8_t NESL_Mapper1RomRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_ROM_CHARACTER:

            switch(address) {
                case 0x0000 ... 0x0FFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[0] + (address & 0x0FFF));
                    break;
                case 0x1000 ... 0x1FFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[1] + (address & 0x0FFF));
                    break;
                default:
                    break;
            }
            break;
        case NESL_BANK_ROM_PROGRAM:

            switch(address) {
                case 0x8000 ... 0xBFFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_PROGRAM, mapper->rom.program[0] + (address & 0x3FFF));
                    break;
                case 0xC000 ... 0xFFFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_PROGRAM, mapper->rom.program[1] + (address & 0x3FFF));
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

void NESL_Mapper1RomWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_ROM_CHARACTER:

            switch(address) {
                case 0x0000 ... 0x0FFF:
                    NESL_CartridgeRamWrite(&mapper->cartridge, NESL_BANK_RAM_CHARACTER, mapper->rom.character[0] + (address & 0x0FFF), data);
                    break;
                case 0x1000 ... 0x1FFF:
                    NESL_CartridgeRamWrite(&mapper->cartridge, NESL_BANK_RAM_CHARACTER, mapper->rom.character[1] + (address & 0x0FFF), data);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BANK_ROM_PROGRAM:

            switch(address) {
                case 0x8000 ... 0xFFFF:
                    NESL_Mapper1Set(mapper, address, data);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void NESL_Mapper1Uninit(nesl_mapper_t *mapper)
{
    memset(&mapper->action, 0, sizeof(mapper->action));

    if(mapper->context) {
        free(mapper->context);
        mapper->context = NULL;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
