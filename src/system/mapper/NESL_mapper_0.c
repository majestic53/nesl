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

#include "../../../include/system/mapper/NESL_mapper_0.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_Mapper0Init(nesl_mapper_t *mapper)
{
    mapper->ram.program = 0;
    mapper->rom.character[0] = 0;
    mapper->rom.program[0] = 0;
    mapper->rom.program[1] = (mapper->cartridge.header->rom.program > 1) ? (16 * 1024) : 0;

    return NESL_SUCCESS;
}

int NESL_Mapper0Interrupt(nesl_mapper_t *mapper)
{
    return NESL_SUCCESS;
}

uint8_t NESL_Mapper0RamRead(nesl_mapper_t *mapper, int type, uint16_t address)
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

void NESL_Mapper0RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
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

int NESL_Mapper0Reset(nesl_mapper_t *mapper)
{
    return NESL_SUCCESS;
}

uint8_t NESL_Mapper0RomRead(nesl_mapper_t *mapper, int type, uint16_t address)
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

void NESL_Mapper0RomWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void NESL_Mapper0Uninit(nesl_mapper_t *mapper)
{
    return;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
