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

#include "../../../include/system/mapper/NESL_mapper_4.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void NESL_Mapper4SetBank(nesl_mapper_t *mapper)
{
    nesl_mapper_4_context_t *context = mapper->context;

    context->bank.index[context->select.bank] = context->bank.data;

    if(context->select.character) {
        mapper->rom.character[0] = context->bank.index[2] * 1 * 1024;
        mapper->rom.character[1] = context->bank.index[3] * 1 * 1024;
        mapper->rom.character[2] = context->bank.index[4] * 1 * 1024;
        mapper->rom.character[3] = context->bank.index[5] * 1 * 1024;
        mapper->rom.character[4] = context->bank.index[0] * 1 * 1024;
        mapper->rom.character[5] = mapper->rom.character[4] + 1024;
        mapper->rom.character[6] = context->bank.index[1] * 1 * 1024;
        mapper->rom.character[7] = mapper->rom.character[6] + 1024;
    } else {
        mapper->rom.character[0] = context->bank.index[0] * 1 * 1024;
        mapper->rom.character[1] = mapper->rom.character[0] + 1024;
        mapper->rom.character[2] = context->bank.index[1] * 1 * 1024;
        mapper->rom.character[3] = mapper->rom.character[2] + 1024;
        mapper->rom.character[4] = context->bank.index[2] * 1 * 1024;
        mapper->rom.character[5] = context->bank.index[3] * 1 * 1024;
        mapper->rom.character[6] = context->bank.index[4] * 1 * 1024;
        mapper->rom.character[7] = context->bank.index[5] * 1 * 1024;
    }

    if(context->select.program) {
        mapper->rom.program[0] = (mapper->cartridge.header->rom.program * 16 * 1024) - (2 * 8 * 1024);
        mapper->rom.program[2] = context->bank.index[6] * 8 * 1024;
    } else {
        mapper->rom.program[0] = context->bank.index[6] * 8 * 1024;
        mapper->rom.program[2] = (mapper->cartridge.header->rom.program * 16 * 1024) - (2 * 8 * 1024);
    }

    mapper->rom.program[1] = context->bank.index[7] * 8 * 1024;
    mapper->rom.program[3] = (mapper->cartridge.header->rom.program * 16 * 1024) - (1 * 8 * 1024);
}

static void NESL_Mapper4SetMirror(nesl_mapper_t *mapper)
{

    if(((nesl_mapper_4_context_t *)mapper->context)->mirror.mode) {
        mapper->mirror = NESL_MIRROR_HORIZONTAL;
    } else {
        mapper->mirror = NESL_MIRROR_VERTICAL;
    }
}

static void NESL_Mapper4Set( nesl_mapper_t *mapper, uint16_t address, uint8_t data)
{
    nesl_mapper_4_context_t *context = mapper->context;

    switch(address) {
        case 0x8000 ... 0x9FFF:

            if(!(address % 2)) {
                context->select.raw = data;
            } else {

                switch(context->select.bank) {
                    case 0 ... 1:
                        data &= 0xFE;
                        break;
                    case 6 ... 7:
                        data &= 0x3F;
                        break;
                    default:
                        break;
                }

                context->bank.data = data;
                NESL_Mapper4SetBank(mapper);
            }
            break;
        case 0xA000 ... 0xBFFF:

            if(!(address % 2)) {
                context->mirror.raw = data;
                NESL_Mapper4SetMirror(mapper);
            } else {
                context->protect.raw = data;
            }
            break;
        case 0xC000 ... 0xDFFF:

            if(!(address % 2)) {
                context->interrupt.latch = data;
            } else {
                context->interrupt.count = 0;
            }
            break;
        case 0xE000 ... 0xFFFF:
            context->interrupt.enable = (address % 2) ? true : false;
            break;
        default:
            break;
    }
}

int NESL_Mapper4Init(nesl_mapper_t *mapper)
{
    int result = NESL_SUCCESS;

    if(!(mapper->context = calloc(1, sizeof(nesl_mapper_4_context_t)))) {
        result = NESL_SET_ERROR("Failed to allocate buffer -- %u KB (%i bytes)", sizeof(nesl_mapper_4_context_t), sizeof(nesl_mapper_4_context_t));
        goto exit;
    }

    if((result = NESL_Mapper4Reset(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int NESL_Mapper4Interrupt(nesl_mapper_t *mapper)
{
    int result = NESL_SUCCESS;

    nesl_mapper_4_context_t *context = mapper->context;

    if(!context->interrupt.count) {
        context->interrupt.count = context->interrupt.latch;
    } else {
        --context->interrupt.count;
    }

    if(!context->interrupt.count && context->interrupt.enable) {

        if((result = NESL_BusInterrupt(NESL_INTERRUPT_MASKABLE)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    return result;
}

uint8_t NESL_Mapper4RamRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_RAM_PROGRAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:

                    if(((nesl_mapper_4_context_t *)mapper->context)->protect.ram_enable) {
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

void NESL_Mapper4RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_RAM_PROGRAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:

                    if(((nesl_mapper_4_context_t *)mapper->context)->protect.ram_enable
                            && !((nesl_mapper_4_context_t *)mapper->context)->protect.ram_read_only) {
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

int NESL_Mapper4Reset(nesl_mapper_t *mapper)
{
    nesl_mapper_4_context_t *context = mapper->context;

    context->protect.ram_enable = true;
    context->protect.ram_read_only = false;
    NESL_Mapper4SetBank(mapper);
    NESL_Mapper4SetMirror(mapper);

    return NESL_SUCCESS;
}

uint8_t NESL_Mapper4RomRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_ROM_CHARACTER:

            switch(address) {
                case 0x0000 ... 0x03FF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[0] + (address & 0x03FF));
                    break;
                case 0x0400 ... 0x07FF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[1] + (address & 0x03FF));
                    break;
                case 0x0800 ... 0x0BFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[2] + (address & 0x03FF));
                    break;
                case 0x0C00 ... 0x0FFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[3] + (address & 0x03FF));
                    break;
                case 0x1000 ... 0x13FF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[4] + (address & 0x03FF));
                    break;
                case 0x1400 ... 0x17FF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[5] + (address & 0x03FF));
                    break;
                case 0x1800 ... 0x1BFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[6] + (address & 0x03FF));
                    break;
                case 0x1C00 ... 0x1FFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[7] + (address & 0x03FF));
                    break;
                default:
                    break;
            }
            break;
        case NESL_BANK_ROM_PROGRAM:

            switch(address) {
                case 0x8000 ... 0x9FFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_PROGRAM, mapper->rom.program[0] + (address & 0x1FFF));
                    break;
                case 0xA000 ... 0xBFFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_PROGRAM, mapper->rom.program[1] + (address & 0x1FFF));
                    break;
                case 0xC000 ... 0xDFFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_PROGRAM, mapper->rom.program[2] + (address & 0x1FFF));
                    break;
                case 0xE000 ... 0xFFFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_PROGRAM, mapper->rom.program[3] + (address & 0x1FFF));
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

void NESL_Mapper4RomWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_ROM_PROGRAM:

            switch(address) {
                case 0x8000 ... 0xFFFF:
                    NESL_Mapper4Set(mapper, address, data);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void NESL_Mapper4Uninit(nesl_mapper_t *mapper)
{

    if(mapper->context) {
        free(mapper->context);
        mapper->context = NULL;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
