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

#include "../../../include/system/mapper/NESL_mapper_66.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static void NESL_Mapper66Set(nesl_mapper_t *mapper)
{
    mapper->rom.character[0] = ((nesl_mapper_66_context_t *)mapper->context)->bank.character * 8 * 1024;
    mapper->rom.program[0] = ((nesl_mapper_66_context_t *)mapper->context)->bank.program * 32 * 1024;
}

int NESL_Mapper66Init(nesl_mapper_t *mapper)
{
    int result = NESL_SUCCESS;

    if(!(mapper->context = calloc(1, sizeof(nesl_mapper_66_context_t)))) {
        result = NESL_SET_ERROR("Failed to allocate buffer -- %u KB (%i bytes)", sizeof(nesl_mapper_66_context_t), sizeof(nesl_mapper_66_context_t));
        goto exit;
    }

    mapper->callback.interrupt = &NESL_Mapper66Interrupt;
    mapper->callback.read_ram = &NESL_Mapper66ReadRam;
    mapper->callback.read_rom = &NESL_Mapper66ReadRom;
    mapper->callback.reset = &NESL_Mapper66Reset;
    mapper->callback.write_ram = &NESL_Mapper66WriteRam;
    mapper->callback.write_rom = &NESL_Mapper66WriteRom;

    if((result = NESL_Mapper66Reset(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

int NESL_Mapper66Interrupt(nesl_mapper_t *mapper)
{
    return NESL_SUCCESS;
}

uint8_t NESL_Mapper66ReadRam(nesl_mapper_t *mapper, int type, uint16_t address)
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

uint8_t NESL_Mapper66ReadRom(nesl_mapper_t *mapper, int type, uint16_t address)
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
                case 0x8000 ... 0xFFFF:
                    result = NESL_CartridgeReadRom(&mapper->cartridge, NESL_BANK_PROGRAM_ROM, mapper->rom.program[0] + (address & 0x7FFF));
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

int NESL_Mapper66Reset(nesl_mapper_t *mapper)
{
    mapper->ram.program = 0;
    NESL_Mapper66Set(mapper);

    return NESL_SUCCESS;
}

void NESL_Mapper66WriteRam(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
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

void NESL_Mapper66WriteRom(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
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
                    ((nesl_mapper_66_context_t *)mapper->context)->bank.raw = data;
                    NESL_Mapper66Set(mapper);
                    break;
                default:
                    break;
            }
            break;
        default:
            break;
    }
}

void NESL_Mapper66Uninit(nesl_mapper_t *mapper)
{
    memset(&mapper->callback, 0, sizeof(mapper->callback));

    if(mapper->context) {
        free(mapper->context);
        mapper->context = NULL;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
