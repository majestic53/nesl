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

#include "../../../include/system/mapper/NESL_mapper_3.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void NESL_Mapper3Set(nesl_mapper_t *mapper)
{
    mapper->rom.character[0] = ((nesl_mapper_3_context_t *)mapper->context)->character.bank * 8 * 1024;
}

int NESL_Mapper3Init(nesl_mapper_t *mapper)
{
    int result = NESL_SUCCESS;

    if(!(mapper->context = calloc(1, sizeof(nesl_mapper_3_context_t)))) {
        result = NESL_SET_ERROR("Failed to allocate buffer -- %u KB (%i bytes)", sizeof(nesl_mapper_3_context_t), sizeof(nesl_mapper_3_context_t));
        goto exit;
    }

    if((result = NESL_Mapper3Reset(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int NESL_Mapper3Interrupt(nesl_mapper_t *mapper)
{
    return NESL_SUCCESS;
}

uint8_t NESL_Mapper3RamRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_RAM_PROGRAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:
                    result = NESL_CartridgeRamRead(&mapper->cartridge, NESL_BANK_RAM_PROGRAM, mapper->ram.program + (address & 0x1FFF));
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

void NESL_Mapper3RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_RAM_PROGRAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:
                    NESL_CartridgeRamWrite(&mapper->cartridge, NESL_BANK_RAM_PROGRAM, mapper->ram.program + (address & 0x1FFF), data);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

int NESL_Mapper3Reset(nesl_mapper_t *mapper)
{
    mapper->ram.program = 0;
    mapper->rom.program[0] = 0;
    mapper->rom.program[1] = (mapper->cartridge.header->rom.program > 1) ? (16 * 1024) : 0;
    NESL_Mapper3Set(mapper);

    return NESL_SUCCESS;
}

uint8_t NESL_Mapper3RomRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_ROM_CHARACTER:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    result = NESL_CartridgeRomRead(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[0] + (address & 0x1FFF));
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

void NESL_Mapper3RomWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_ROM_PROGRAM:

            switch(address) {
                case 0x8000 ... 0xFFFF:
                    ((nesl_mapper_3_context_t *)mapper->context)->character.raw = data;
                    NESL_Mapper3Set(mapper);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void NESL_Mapper3Uninit(nesl_mapper_t *mapper)
{

    if(mapper->context) {
        free(mapper->context);
        mapper->context = NULL;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
