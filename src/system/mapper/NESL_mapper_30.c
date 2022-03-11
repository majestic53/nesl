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
 * @file NESL_mapper_30.c
 * @brief Mapper 30 (UNROM) extension.
 */

#include <NESL_mapper_30.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Set mapper-30 charcter/program ROM banks and mirror type.
 * @param mapper Pointer to mapper subsystem context
 */
static void NESL_Mapper30Set(nesl_mapper_t *mapper)
{
    mapper->rom.program[0] = ((nesl_mapper_30_t *)mapper->context)->bank.program * 16 * 1024;
    mapper->rom.character[0] = ((nesl_mapper_30_t *)mapper->context)->bank.character * 8 * 1024;
    mapper->mirror = ((nesl_mapper_30_t *)mapper->context)->bank.one_screen ? NESL_MIRROR_ONE_LOW : NESL_CartridgeGetMirror(&mapper->cartridge);
}

nesl_error_e NESL_Mapper30Initialize(nesl_mapper_t *mapper)
{
    nesl_error_e result = NESL_SUCCESS;

    if(!(mapper->context = calloc(1, sizeof(nesl_mapper_30_t)))) {
        result = NESL_SET_ERROR("Failed to allocate buffer -- %u KB (%i bytes)", sizeof(nesl_mapper_30_t), sizeof(nesl_mapper_30_t));
        goto exit;
    }

    mapper->extension.interrupt = &NESL_Mapper30Interrupt;
    mapper->extension.read_ram = &NESL_Mapper30ReadRam;
    mapper->extension.read_rom = &NESL_Mapper30ReadRom;
    mapper->extension.reset = &NESL_Mapper30Reset;
    mapper->extension.write_ram = &NESL_Mapper30WriteRam;
    mapper->extension.write_rom = &NESL_Mapper30WriteRom;

    if((result = NESL_Mapper30Reset(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

nesl_error_e NESL_Mapper30Interrupt(nesl_mapper_t *mapper)
{
    return NESL_SUCCESS;
}

uint8_t NESL_Mapper30ReadRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
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

uint8_t NESL_Mapper30ReadRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
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

nesl_error_e NESL_Mapper30Reset(nesl_mapper_t *mapper)
{
    mapper->rom.program[1] = (NESL_CartridgeGetBankCount(&mapper->cartridge, NESL_BANK_PROGRAM_ROM) * 16 * 1024) - (1 * 16 * 1024);
    NESL_Mapper30Set(mapper);

    return NESL_SUCCESS;
}

void NESL_Mapper30WriteRam(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
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

void NESL_Mapper30Uninitialize(nesl_mapper_t *mapper)
{
    memset(&mapper->extension, 0, sizeof(mapper->extension));

    if(mapper->context) {
        free(mapper->context);
        mapper->context = NULL;
    }
}

void NESL_Mapper30WriteRom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_CHARACTER_ROM:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    NESL_CartridgeWriteRam(&mapper->cartridge, NESL_BANK_CHARACTER_RAM, mapper->rom.character[0] + (address & 0x1FFF), data);
                    break;
                default:
                    break;
            }
            break;
        case NESL_BANK_PROGRAM_ROM:

            switch(address) {
                case 0x8000 ... 0xFFFF:
                    ((nesl_mapper_30_t *)mapper->context)->bank.raw = data;
                    NESL_Mapper30Set(mapper);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
