/*
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

/*!
 * @file mapper_3.c
 * @brief Mapper 3 (CNROM) extension.
 */

#include <mapper_3.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Set mapper-3 charcter ROM bank.
 * @param[in,out] mapper Pointer to mapper subsystem context
 */
static void nesl_mapper_3_set(nesl_mapper_t *mapper)
{
    mapper->rom.character[0] = ((nesl_mapper_3_t *)mapper->context)->character.bank * 8 * 1024;
}

nesl_error_e nesl_mapper_3_initialize(nesl_mapper_t *mapper)
{
    nesl_error_e result = NESL_SUCCESS;

    if(!(mapper->context = calloc(1, sizeof(nesl_mapper_3_t)))) {
        result = SET_ERROR("Failed to allocate buffer -- %u KB (%i bytes)", sizeof(nesl_mapper_3_t), sizeof(nesl_mapper_3_t));
        goto exit;
    }

    mapper->extension.interrupt = &nesl_mapper_3_interrupt;
    mapper->extension.read_ram = &nesl_mapper_3_read_ram;
    mapper->extension.read_rom = &nesl_mapper_3_read_rom;
    mapper->extension.reset = &nesl_mapper_3_reset;
    mapper->extension.write_ram = &nesl_mapper_3_write_ram;
    mapper->extension.write_rom = &nesl_mapper_3_write_rom;

    if((result = nesl_mapper_3_reset(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

nesl_error_e nesl_mapper_3_interrupt(nesl_mapper_t *mapper)
{
    return NESL_SUCCESS;
}

uint8_t nesl_mapper_3_read_ram(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case BANK_PROGRAM_RAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:
                    result = nesl_cartridge_read_ram(&mapper->cartridge, BANK_PROGRAM_RAM, mapper->ram.program + (address & 0x1FFF));
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

uint8_t nesl_mapper_3_read_rom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case BANK_CHARACTER_ROM:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    result = nesl_cartridge_read_rom(&mapper->cartridge, BANK_CHARACTER_ROM, mapper->rom.character[0] + (address & 0x1FFF));
                    break;
                default:
                    break;
            }
            break;
        case BANK_PROGRAM_ROM:

            switch(address) {
                case 0x8000 ... 0xBFFF:
                    result = nesl_cartridge_read_rom(&mapper->cartridge, BANK_PROGRAM_ROM, mapper->rom.program[0] + (address & 0x3FFF));
                    break;
                case 0xC000 ... 0xFFFF:
                    result = nesl_cartridge_read_rom(&mapper->cartridge, BANK_PROGRAM_ROM, mapper->rom.program[1] + (address & 0x3FFF));
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

nesl_error_e nesl_mapper_3_reset(nesl_mapper_t *mapper)
{
    mapper->ram.program = 0;
    mapper->rom.program[0] = 0;
    mapper->rom.program[1] = (nesl_cartridge_get_banks(&mapper->cartridge, BANK_PROGRAM_ROM) > 1) ? (16 * 1024) : 0;
    nesl_mapper_3_set(mapper);

    return NESL_SUCCESS;
}

void nesl_mapper_3_uninitialize(nesl_mapper_t *mapper)
{
    memset(&mapper->extension, 0, sizeof(mapper->extension));

    if(mapper->context) {
        free(mapper->context);
        mapper->context = NULL;
    }
}

void nesl_mapper_3_write_ram(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{

    switch(type) {
        case BANK_PROGRAM_RAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:
                    nesl_cartridge_write_ram(&mapper->cartridge, BANK_PROGRAM_RAM, mapper->ram.program + (address & 0x1FFF), data);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void nesl_mapper_3_write_rom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{

    switch(type) {
        case BANK_PROGRAM_ROM:

            switch(address) {
                case 0x8000 ... 0xFFFF:
                    ((nesl_mapper_3_t *)mapper->context)->character.raw = data;
                    nesl_mapper_3_set(mapper);
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
