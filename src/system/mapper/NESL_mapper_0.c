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

/**
 * @file NESL_mapper_0.c
 * @brief Mapper 0 (NROM) extension.
 */

#include "../../../include/system/mapper/NESL_mapper_0.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

nesl_error_e NESL_Mapper0Initialize(nesl_mapper_t *mapper)
{
    mapper->extension.interrupt = &NESL_Mapper0Interrupt;
    mapper->extension.read_ram = &NESL_Mapper0ReadRam;
    mapper->extension.read_rom = &NESL_Mapper0ReadRom;
    mapper->extension.reset = &NESL_Mapper0Reset;
    mapper->extension.write_ram = &NESL_Mapper0WriteRam;
    mapper->extension.write_rom = &NESL_Mapper0WriteRom;
    mapper->ram.program = 0;
    mapper->rom.character[0] = 0;
    mapper->rom.program[0] = 0;
    mapper->rom.program[1] = (NESL_CartridgeGetBankCount(&mapper->cartridge, NESL_BANK_PROGRAM_ROM) > 1) ? (16 * 1024) : 0;

    return NESL_SUCCESS;
}

nesl_error_e NESL_Mapper0Interrupt(nesl_mapper_t *mapper)
{
    return NESL_SUCCESS;
}

uint8_t NESL_Mapper0ReadRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_PROGRAM_RAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:
                    result = NESL_CartridgeReadRam(&mapper->cartridge, NESL_BANK_PROGRAM_RAM, mapper->ram.program + (address & 0x1FFF));
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

uint8_t NESL_Mapper0ReadRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_CHARACTER_ROM:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    result = NESL_CartridgeReadRom(&mapper->cartridge, NESL_BANK_CHARACTER_ROM, mapper->rom.character[0] + (address & 0x1FFF));
                    break;
                default:
                    break;
            }
            break;
        case NESL_BANK_PROGRAM_ROM:

            switch(address) {
                case 0x8000 ... 0xBFFF:
                    result = NESL_CartridgeReadRom(&mapper->cartridge, NESL_BANK_PROGRAM_ROM, mapper->rom.program[0] + (address & 0x3FFF));
                    break;
                case 0xC000 ... 0xFFFF:
                    result = NESL_CartridgeReadRom(&mapper->cartridge, NESL_BANK_PROGRAM_ROM, mapper->rom.program[1] + (address & 0x3FFF));
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

nesl_error_e NESL_Mapper0Reset(nesl_mapper_t *mapper)
{
    return NESL_SUCCESS;
}

void NESL_Mapper0Uninitialize(nesl_mapper_t *mapper)
{
    memset(&mapper->extension, 0, sizeof(mapper->extension));
}

void NESL_Mapper0WriteRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_PROGRAM_RAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:
                    NESL_CartridgeWriteRam(&mapper->cartridge, NESL_BANK_PROGRAM_RAM, mapper->ram.program + (address & 0x1FFF), data);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void NESL_Mapper0WriteRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{
    return;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
