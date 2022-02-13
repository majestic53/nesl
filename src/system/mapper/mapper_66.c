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

#include "../../../include/system/mapper/mapper_66.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void nesl_mapper_66_set(nesl_mapper_t *mapper)
{
    mapper->rom.character[0] = ((nesl_mapper_66_context_t *)mapper->context)->bank.character * 8 * 1024;
    mapper->rom.program[0] = ((nesl_mapper_66_context_t *)mapper->context)->bank.program * 32 * 1024;
}

int nesl_mapper_66_initialize(nesl_mapper_t *mapper)
{
    int result = NESL_SUCCESS;

    if(!(mapper->context = calloc(1, sizeof(nesl_mapper_66_context_t)))) {
        result = NESL_ERROR_SET("Failed to allocate buffer -- %u KB (%i bytes)", sizeof(nesl_mapper_66_context_t), sizeof(nesl_mapper_66_context_t));
        goto exit;
    }

    if((result = nesl_mapper_66_reset(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

uint8_t nesl_mapper_66_ram_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_RAM_PROGRAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:
                    result = nesl_cartridge_ram_read(&mapper->cartridge, NESL_BANK_RAM_PROGRAM, mapper->ram.program + (address & 0x1FFF));
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

void nesl_mapper_66_ram_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_RAM_PROGRAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:
                    nesl_cartridge_ram_write(&mapper->cartridge, NESL_BANK_RAM_PROGRAM, mapper->ram.program + (address & 0x1FFF), data);
                    break;
                default:
                    break;
            }

            break;
        default:
            break;
    }
}

int nesl_mapper_66_reset(nesl_mapper_t *mapper)
{
    mapper->ram.program = 0;
    nesl_mapper_66_set(mapper);

    return NESL_SUCCESS;
}

uint8_t nesl_mapper_66_rom_read(nesl_mapper_t *mapper, int type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case NESL_BANK_ROM_CHARACTER:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    result = nesl_cartridge_rom_read(&mapper->cartridge, NESL_BANK_ROM_CHARACTER, mapper->rom.character[0] + (address & 0x1FFF));
                    break;
                default:
                    break;
            }

            break;
        case NESL_BANK_ROM_PROGRAM:

            switch(address) {
                case 0x8000 ... 0xFFFF:
                    result = nesl_cartridge_rom_read(&mapper->cartridge, NESL_BANK_ROM_PROGRAM, mapper->rom.program[0] + (address & 0x7FFF));
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

void nesl_mapper_66_rom_write(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{

    switch(type) {
        case NESL_BANK_ROM_CHARACTER:

            switch(address) {
                case 0x0000 ... 0x1FFF:
                    nesl_cartridge_ram_write(&mapper->cartridge, NESL_BANK_RAM_CHARACTER, mapper->rom.character[0] + (address & 0x1FFF), data);
                    break;
                default:
                    break;
            }

            break;
        case NESL_BANK_ROM_PROGRAM:

            switch(address) {
                case 0x8000 ... 0xFFFF:
                    ((nesl_mapper_66_context_t *)mapper->context)->bank.raw = data;
                    nesl_mapper_66_set(mapper);
                    break;
                default:
                    break;
            }

            break;
        default:
            break;
    }
}

void nesl_mapper_66_uninitialize(nesl_mapper_t *mapper)
{

    if(mapper->context) {
        free(mapper->context);
        mapper->context = NULL;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
