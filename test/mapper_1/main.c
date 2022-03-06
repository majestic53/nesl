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
 * @file main.c
 * @brief Test application for mapper 1 extension.
 */

#include "../../include/system/mapper/NESL_mapper_1.h"
#include "../include/NESL_common.h"

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_mapper_t mapper;   /*< Mapper type */
    nesl_bank_e type;       /*< Bank type */
    uint32_t address;       /*< Bank address */
    uint8_t data;           /*< Bank data */

    struct {

        struct {
            uint8_t character[8 * 1024];    /*< Character RAM */
            uint8_t program[8 * 1024];      /*< Program RAM */
        } ram;

        struct {
            uint8_t character[8 * 1024];    /*< Character ROM */
            uint8_t program[2 * 16 * 1024]; /*< Program ROM */
        } rom;
    } cartridge;
} nesl_test_t;

static nesl_test_t g_test = {}; /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uint8_t NESL_CartridgeGetBankCount(nesl_cartridge_t *cartridge, nesl_bank_e type)
{
    return g_test.mapper.cartridge.header->rom.program;
}

uint8_t NESL_CartridgeReadRam(nesl_cartridge_t *cartridge, nesl_bank_e type, uint32_t address)
{
    g_test.address = address;

    switch((g_test.type = type)) {
        case NESL_BANK_PROGRAM_RAM:
            g_test.data = g_test.cartridge.ram.program[g_test.address & 0xFFFF];
            break;
        default:
            break;
    }

    return g_test.data;
}

uint8_t NESL_CartridgeReadRom(nesl_cartridge_t *cartridge, nesl_bank_e type, uint32_t address)
{
    g_test.address = address;

    switch((g_test.type = type)) {
        case NESL_BANK_CHARACTER_ROM:
            g_test.data = g_test.cartridge.rom.character[g_test.address & 0xFFFF];
            break;
        case NESL_BANK_PROGRAM_ROM:
            g_test.data = g_test.cartridge.rom.program[g_test.address & 0xFFFF];
            break;
        default:
            break;
    }

    return g_test.data;
}

void NESL_CartridgeWriteRam(nesl_cartridge_t *cartridge, nesl_bank_e type, uint32_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;

    switch((g_test.type = type)) {
        case NESL_BANK_CHARACTER_RAM:
            g_test.cartridge.ram.character[g_test.address & 0xFFFF] = g_test.data;
            break;
        case NESL_BANK_PROGRAM_RAM:
            g_test.cartridge.ram.program[g_test.address & 0xFFFF] = g_test.data;
            break;
        default:
            break;
    }
}

nesl_error_e NESL_SetError(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

static void NESL_TestUninit(void)
{
    NESL_Mapper1Uninit(&g_test.mapper);
    g_test.mapper.callback.interrupt = NULL;
    g_test.mapper.callback.read_ram = NULL;
    g_test.mapper.callback.read_rom = NULL;
    g_test.mapper.callback.reset = NULL;
    g_test.mapper.callback.write_ram = NULL;
    g_test.mapper.callback.write_rom = NULL;
}

static nesl_error_e NESL_TestInit(const nesl_cartridge_header_t *header)
{
    NESL_TestUninit();
    memset(&g_test, 0, sizeof(g_test));
    g_test.mapper.cartridge.header = header;
    g_test.mapper.callback.interrupt = &NESL_Mapper1Interrupt;
    g_test.mapper.callback.read_ram = &NESL_Mapper1ReadRam;
    g_test.mapper.callback.read_rom = &NESL_Mapper1ReadRom;
    g_test.mapper.callback.reset = &NESL_Mapper1Reset;
    g_test.mapper.callback.write_ram = &NESL_Mapper1WriteRam;
    g_test.mapper.callback.write_rom = &NESL_Mapper1WriteRom;

    return NESL_Mapper1Init(&g_test.mapper);
}

static nesl_error_e NESL_TestMapper1Init(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    header.rom.program = 1;
    header.rom.character = 2;

    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((g_test.mapper.ram.program == 0)
            && (g_test.mapper.rom.character[0] == 0)
            && (g_test.mapper.rom.character[1] == 4 * 1024)
            && (g_test.mapper.rom.program[0] == 0)
            && (g_test.mapper.rom.program[1] == 0)
            && (g_test.mapper.mirror == NESL_MIRROR_ONE_LOW)
            && (g_test.mapper.context != NULL)
            && (g_test.mapper.callback.interrupt == &NESL_Mapper1Interrupt)
            && (g_test.mapper.callback.read_ram == &NESL_Mapper1ReadRam)
            && (g_test.mapper.callback.read_rom == &NESL_Mapper1ReadRom)
            && (g_test.mapper.callback.reset == &NESL_Mapper1Reset)
            && (g_test.mapper.callback.write_ram == &NESL_Mapper1WriteRam)
            && (g_test.mapper.callback.write_rom == &NESL_Mapper1WriteRom))) {
        result = NESL_FAILURE;
        goto exit;
    }

    header.rom.program = 4;
    header.rom.character = 2;

    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((g_test.mapper.ram.program == 0)
            && (g_test.mapper.rom.character[0] == 0)
            && (g_test.mapper.rom.character[1] == 4 * 1024)
            && (g_test.mapper.rom.program[0] == 0)
            && (g_test.mapper.rom.program[1] == 3 * 16 * 1024)
            && (g_test.mapper.mirror == NESL_MIRROR_ONE_LOW)
            && (g_test.mapper.context != NULL)
            && (g_test.mapper.callback.interrupt == &NESL_Mapper1Interrupt)
            && (g_test.mapper.callback.read_ram == &NESL_Mapper1ReadRam)
            && (g_test.mapper.callback.read_rom == &NESL_Mapper1ReadRom)
            && (g_test.mapper.callback.reset == &NESL_Mapper1Reset)
            && (g_test.mapper.callback.write_ram == &NESL_Mapper1WriteRam)
            && (g_test.mapper.callback.write_rom == &NESL_Mapper1WriteRom))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestMapper1Interrupt(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_Mapper1Interrupt(&g_test.mapper) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestMapper1ReadRam(void)
{
    uint8_t data = 0;
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x6000 ... 0x7FFF:

                for(nesl_bank_e type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

                    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    if(type == NESL_BANK_PROGRAM_RAM) {
                        g_test.cartridge.ram.program[address & 0x1FFF] = data;

                        if(NESL_ASSERT((NESL_Mapper1ReadRam(&g_test.mapper, type, address) == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }

                        ((nesl_mapper_1_context_t *)g_test.mapper.context)->program.ram_disable = true;

                        if(NESL_ASSERT((NESL_Mapper1ReadRam(&g_test.mapper, type, address) == 0)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }

                        ((nesl_mapper_1_context_t *)g_test.mapper.context)->program.ram_disable = false;
                    } else if(NESL_ASSERT((g_test.data == 0)
                            && (g_test.type == 0)
                            && (g_test.address == 0))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;
            default:

                if(NESL_ASSERT((g_test.data == 0)
                        && (g_test.type == 0)
                        && (g_test.address == 0))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestMapper1ReadRom(void)
{
    uint8_t data = 0;
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x0000 ... 0x1FFF:

                for(nesl_bank_e type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

                    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    if(type == NESL_BANK_CHARACTER_ROM) {
                        g_test.cartridge.rom.character[address & 0x1FFF] = data;

                        if(NESL_ASSERT((NESL_Mapper1ReadRom(&g_test.mapper, type, address) == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    } else if(NESL_ASSERT((g_test.data == 0)
                            && (g_test.type == 0)
                            && (g_test.address == 0))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;
            case 0x8000 ... 0xFFFF:
                for(nesl_bank_e type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

                    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    if(type == NESL_BANK_PROGRAM_ROM) {
                        g_test.cartridge.rom.program[address & 0x7FFF] = data;

                        if(NESL_ASSERT((NESL_Mapper1ReadRom(&g_test.mapper, type, address) == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x7FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    } else if(NESL_ASSERT((g_test.data == 0)
                            && (g_test.type == 0)
                            && (g_test.address == 0))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;
            default:

                if(NESL_ASSERT((g_test.data == 0)
                        && (g_test.type == 0)
                        && (g_test.address == 0))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestMapper1Reset(void)
{
    nesl_mapper_1_context_t *context;
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_Mapper1Reset(&g_test.mapper) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    context = (nesl_mapper_1_context_t *)g_test.mapper.context;

    if(NESL_ASSERT((context->control.raw == 0x0C)
            && (g_test.mapper.mirror == NESL_MIRROR_ONE_LOW)
            && (g_test.mapper.rom.program[0] == context->program.bank * 16 * 1024)
            && (g_test.mapper.rom.program[1] == (g_test.mapper.cartridge.header->rom.program - 1) * 16 * 1024)
            && (g_test.mapper.rom.character[0] == (context->character[0].bank >> 1) * 8 * 1024)
            && (g_test.mapper.rom.character[1] == g_test.mapper.rom.character[0] + (4 * 1024)))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestMapper1WriteRam(void)
{
    uint8_t data = 0;
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x6000 ... 0x7FFF:

                for(nesl_bank_e type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

                    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    NESL_Mapper1WriteRam(&g_test.mapper, type, address, data);

                    if(type == NESL_BANK_PROGRAM_RAM) {

                        if(NESL_ASSERT((g_test.data == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }

                        ((nesl_mapper_1_context_t *)g_test.mapper.context)->program.ram_disable = true;

                        NESL_Mapper1WriteRam(&g_test.mapper, type, address, data + 1);

                        if(NESL_ASSERT((g_test.data == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }

                        ((nesl_mapper_1_context_t *)g_test.mapper.context)->program.ram_disable = false;
                    } else if(NESL_ASSERT((g_test.data == 0)
                            && (g_test.type == 0)
                            && (g_test.address == 0))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;
            default:

                if(NESL_ASSERT((g_test.data == 0)
                        && (g_test.type == 0)
                        && (g_test.address == 0))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestMapper1WriteRom(void)
{
    uint8_t data = 0;
    nesl_error_e result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {
        nesl_mapper_1_control_t control = {};
        nesl_mapper_1_program_t program = {};
        nesl_mapper_1_context_t *context = NULL;
        nesl_mapper_1_character_t character = {};
        nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

        switch(address) {
            case 0x0000 ... 0x1FFF:

                for(nesl_bank_e type = 0; type < NESL_BANK_MAX; ++type) {

                    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    g_test.mapper.cartridge.rom.character = g_test.cartridge.ram.character;
                    NESL_Mapper1WriteRom(&g_test.mapper, type, address, data);

                    if(type == NESL_BANK_CHARACTER_ROM) {

                        if(NESL_ASSERT((g_test.data == data)
                                && (g_test.type == NESL_BANK_CHARACTER_RAM)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    } else if(NESL_ASSERT((g_test.data == 0)
                            && (g_test.type == 0)
                            && (g_test.address == 0))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    g_test.mapper.cartridge.rom.character = g_test.cartridge.rom.character;
                }
                break;
            case 0x8000 ... 0x9FFF:

                if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                    goto exit;
                }

                context = g_test.mapper.context;
                context->shift.data = 0xFF;
                context->shift.position = 3;

                NESL_Mapper1WriteRom(&g_test.mapper, NESL_BANK_PROGRAM_ROM, address, 0x80);

                if(NESL_ASSERT((context->shift.data == 0)
                        && (context->shift.position == 0))) {
                    result = NESL_FAILURE;
                    goto exit;
                }

                control.mirror = 3;
                control.program = 2;
                control.character = 1;
                data = control.raw;

                for(uint8_t index = 1; index < 5; ++index) {
                    NESL_Mapper1WriteRom(&g_test.mapper, NESL_BANK_PROGRAM_ROM, address, data & 1);

                    if(NESL_ASSERT(context->shift.position == index)) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    data >>= 1;
                }

                context->character[0].bank = 1;
                context->character[1].bank = 2;
                context->program.bank = 1;
                NESL_Mapper1WriteRom(&g_test.mapper, NESL_BANK_PROGRAM_ROM, address, data & 1);

                if(NESL_ASSERT((context->control.raw == control.raw)
                        && (context->shift.data == 0)
                        && (context->shift.position == 0)
                        && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL)
                        && (g_test.mapper.rom.character[0] == (context->character[0].bank * 4 * 1024))
                        && (g_test.mapper.rom.character[1] == (context->character[1].bank * 4 * 1024))
                        && (g_test.mapper.rom.program[0] == 0x0000)
                        && (g_test.mapper.rom.program[1] == (context->program.bank * 16 * 1024)))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0xA000 ... 0xDFFF:

                if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                    goto exit;
                }

                context = g_test.mapper.context;
                context->shift.data = 0xFF;
                context->shift.position = 3;

                NESL_Mapper1WriteRom(&g_test.mapper, NESL_BANK_PROGRAM_ROM, address, 0x80);
                character.bank = (address >= 0xC000) ? 4 : 6;
                data = character.raw;

                for(uint8_t index = 1; index < 5; ++index) {
                    NESL_Mapper1WriteRom(&g_test.mapper, NESL_BANK_PROGRAM_ROM, address, data & 1);

                    if(NESL_ASSERT(context->shift.position == index)) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    data >>= 1;
                }

                NESL_Mapper1WriteRom(&g_test.mapper, NESL_BANK_PROGRAM_ROM, address, data & 1);

                if(NESL_ASSERT(context->character[(address >= 0xC000) ? 1 : 0].raw == character.raw)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            case 0xE000 ... 0xFFFF:

                if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                    goto exit;
                }

                context = g_test.mapper.context;
                context->shift.data = 0xFF;
                context->shift.position = 3;

                NESL_Mapper1WriteRom(&g_test.mapper, NESL_BANK_PROGRAM_ROM, address, 0x80);
                program.bank = 8;
                data = program.raw;

                for(uint8_t index = 1; index < 5; ++index) {
                    NESL_Mapper1WriteRom(&g_test.mapper, NESL_BANK_PROGRAM_ROM, address, data & 1);

                    if(NESL_ASSERT(context->shift.position == index)) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    data >>= 1;
                }

                NESL_Mapper1WriteRom(&g_test.mapper, NESL_BANK_PROGRAM_ROM, address, data & 1);

                if(NESL_ASSERT(context->program.raw == program.raw)) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            default:

                if(NESL_ASSERT((g_test.data == 0)
                        && (g_test.type == 0)
                        && (g_test.address == 0))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestMapper1Uninit(void)
{
    nesl_error_e result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
        goto exit;
    }

    NESL_Mapper1Uninit(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper.context == NULL)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

int main(void)
{
    static const NESL_Test TEST[] = {
        NESL_TestMapper1Init, NESL_TestMapper1Interrupt, NESL_TestMapper1ReadRam, NESL_TestMapper1ReadRom,
        NESL_TestMapper1Reset, NESL_TestMapper1WriteRam, NESL_TestMapper1WriteRom, NESL_TestMapper1Uninit,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < NESL_TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
