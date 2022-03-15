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
 * @file mapper_1.c
 * @brief Mapper 1 (MMC1) extension.
 */

#include <mapper_1.h>

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Set mapper-1 character ROM bank offsets.
 * @param[in,out] mapper Pointer to mapper subsystem context
 */
static void nesl_mapper_1_set_character(nesl_mapper_t *mapper)
{
    const nesl_mapper_1_t *context = mapper->context;

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

/*!
 * @brief Set mapper-1 mirror type.
 * @param[in,out] mapper Pointer to mapper subsystem context
 */
static void nesl_mapper_1_set_mirror(nesl_mapper_t *mapper)
{

    switch(((nesl_mapper_1_t *)mapper->context)->control.mirror) {
        case 0:
            mapper->mirror = MIRROR_ONE_LOW;
            break;
        case 1:
            mapper->mirror = MIRROR_ONE_HIGH;
            break;
        case 2:
            mapper->mirror = MIRROR_VERTICAL;
            break;
        case 3:
            mapper->mirror = MIRROR_HORIZONTAL;
            break;
        default:
            break;
    }
}

/*!
 * @brief Set mapper-1 program ROM bank offsets.
 * @param[in,out] mapper Pointer to mapper subsystem context
 */
static void nesl_mapper_1_set_program(nesl_mapper_t *mapper)
{
    const nesl_mapper_1_t *context = mapper->context;

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
            mapper->rom.program[1] = (nesl_cartridge_get_banks(&mapper->cartridge, BANK_PROGRAM_ROM) - 1) * 16 * 1024;
            break;
        default:
            break;
    }
}

/*!
 * @brief Set mapper-1 shift register and state.
 * @param[in,out] mapper Pointer to mapper subsystem context
 * @param[in] address mapper address
 * @param[in] data mapper data
 */
static void nesl_mapper_1_set(nesl_mapper_t *mapper, uint16_t address, uint8_t data)
{
    nesl_mapper_1_t *context = mapper->context;

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

nesl_error_e nesl_mapper_1_initialize(nesl_mapper_t *mapper)
{
    nesl_error_e result = NESL_SUCCESS;

    if(!(mapper->context = calloc(1, sizeof(nesl_mapper_1_t)))) {
        result = SET_ERROR("Failed to allocate buffer -- %u KB (%i bytes)", sizeof(nesl_mapper_1_t), sizeof(nesl_mapper_1_t));
        goto exit;
    }

    mapper->extension.interrupt = &nesl_mapper_1_interrupt;
    mapper->extension.read_ram = &nesl_mapper_1_read_ram;
    mapper->extension.read_rom = &nesl_mapper_1_read_rom;
    mapper->extension.reset = &nesl_mapper_1_reset;
    mapper->extension.write_ram = &nesl_mapper_1_write_ram;
    mapper->extension.write_rom = &nesl_mapper_1_write_rom;

    if((result = nesl_mapper_1_reset(mapper)) == NESL_FAILURE) {
        goto exit;
    }

exit:
    return result;
}

nesl_error_e nesl_mapper_1_interrupt(nesl_mapper_t *mapper)
{
    return NESL_SUCCESS;
}

uint8_t nesl_mapper_1_read_ram(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case BANK_PROGRAM_RAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:

                    if(!((nesl_mapper_1_t *)mapper->context)->program.ram_disable) {
                        result = nesl_cartridge_read_ram(&mapper->cartridge, BANK_PROGRAM_RAM, mapper->ram.program + (address & 0x1FFF));
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

uint8_t nesl_mapper_1_read_rom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address)
{
    uint8_t result = 0;

    switch(type) {
        case BANK_CHARACTER_ROM:

            switch(address) {
                case 0x0000 ... 0x0FFF:
                    result = nesl_cartridge_read_rom(&mapper->cartridge, BANK_CHARACTER_ROM, mapper->rom.character[0] + (address & 0x0FFF));
                    break;
                case 0x1000 ... 0x1FFF:
                    result = nesl_cartridge_read_rom(&mapper->cartridge, BANK_CHARACTER_ROM, mapper->rom.character[1] + (address & 0x0FFF));
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

nesl_error_e nesl_mapper_1_reset(nesl_mapper_t *mapper)
{
    ((nesl_mapper_1_t *)mapper->context)->control.raw = 0x0C;
    nesl_mapper_1_set_mirror(mapper);
    nesl_mapper_1_set_character(mapper);
    nesl_mapper_1_set_program(mapper);

    return NESL_SUCCESS;
}

void nesl_mapper_1_uninitialize(nesl_mapper_t *mapper)
{
    memset(&mapper->extension, 0, sizeof(mapper->extension));

    if(mapper->context) {
        free(mapper->context);
        mapper->context = NULL;
    }
}

void nesl_mapper_1_write_ram(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{

    switch(type) {
        case BANK_PROGRAM_RAM:

            switch(address) {
                case 0x6000 ... 0x7FFF:

                    if(!((nesl_mapper_1_t *)mapper->context)->program.ram_disable) {
                        nesl_cartridge_write_ram(&mapper->cartridge, BANK_PROGRAM_RAM, mapper->ram.program + (address & 0x1FFF), data);
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

void nesl_mapper_1_write_rom(nesl_mapper_t *mapper, nesl_bank_e type, uint16_t address, uint8_t data)
{

    switch(type) {
        case BANK_CHARACTER_ROM:

            switch(address) {
                case 0x0000 ... 0x0FFF:
                    nesl_cartridge_write_ram(&mapper->cartridge, BANK_CHARACTER_RAM, mapper->rom.character[0] + (address & 0x0FFF), data);
                    break;
                case 0x1000 ... 0x1FFF:
                    nesl_cartridge_write_ram(&mapper->cartridge, BANK_CHARACTER_RAM, mapper->rom.character[1] + (address & 0x0FFF), data);
                    break;
                default:
                    break;
            }
            break;
        case BANK_PROGRAM_ROM:

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

#ifdef __cplusplus
}
#endif /* __cplusplus */
