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

#include "../../include/system/mapper/NESL_mapper_4.h"
#include "../include/NESL_common.h"

typedef struct {
    nesl_mapper_t mapper;
    int type;
    uint32_t address;
    uint8_t data;
    int int_type;

    struct {

        struct {
            uint8_t character[8 * 1024];
            uint8_t program[8 * 1024];
        } ram;

        struct {
            uint8_t character[8 * 1024];
            uint8_t program[2 * 16 * 1024];
        } rom;
    } cartridge;
} nesl_test_t;

static nesl_test_t g_test = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_BusInterrupt(int type)
{
    g_test.int_type = type;

    return NESL_SUCCESS;
}

uint8_t NESL_CartridgeGetBankCount(nesl_cartridge_t *cartridge, int type)
{
    return g_test.mapper.cartridge.header->rom.program;
}

uint8_t NESL_CartridgeRamRead(nesl_cartridge_t *cartridge, int type, uint32_t address)
{
    g_test.address = address;

    switch((g_test.type = type)) {
        case NESL_BANK_RAM_PROGRAM:
            g_test.data = g_test.cartridge.ram.program[g_test.address & 0xFFFF];
            break;
        default:
            break;
    }

    return g_test.data;
}

void NESL_CartridgeRamWrite(nesl_cartridge_t *cartridge, int type, uint32_t address, uint8_t data)
{
    g_test.address = address;
    g_test.data = data;

    switch((g_test.type = type)) {
        case NESL_BANK_RAM_CHARACTER:
            g_test.cartridge.ram.character[g_test.address & 0xFFFF] = g_test.data;
            break;
        case NESL_BANK_RAM_PROGRAM:
            g_test.cartridge.ram.program[g_test.address & 0xFFFF] = g_test.data;
            break;
        default:
            break;
    }
}

uint8_t NESL_CartridgeRomRead(nesl_cartridge_t *cartridge, int type, uint32_t address)
{
    g_test.address = address;

    switch((g_test.type = type)) {
        case NESL_BANK_ROM_CHARACTER:
            g_test.data = g_test.cartridge.rom.character[g_test.address & 0xFFFF];
            break;
        case NESL_BANK_ROM_PROGRAM:
            g_test.data = g_test.cartridge.rom.program[g_test.address & 0xFFFF];
            break;
        default:
            break;
    }

    return g_test.data;
}

int NESL_SetError(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

static void NESL_TestUninit(void)
{
    NESL_Mapper4Uninit(&g_test.mapper);
    g_test.mapper.action.interrupt = NULL;
    g_test.mapper.action.ram_read = NULL;
    g_test.mapper.action.ram_write = NULL;
    g_test.mapper.action.reset = NULL;
    g_test.mapper.action.rom_read = NULL;
    g_test.mapper.action.rom_write = NULL;
}

static int NESL_TestInit(const nesl_cartridge_header_t *header)
{
    NESL_TestUninit();
    memset(&g_test, 0, sizeof(g_test));
    g_test.mapper.cartridge.header = header;
    g_test.mapper.action.interrupt = &NESL_Mapper4Interrupt;
    g_test.mapper.action.ram_read = &NESL_Mapper4RamRead;
    g_test.mapper.action.ram_write = &NESL_Mapper4RamWrite;
    g_test.mapper.action.reset = &NESL_Mapper4Reset;
    g_test.mapper.action.rom_read = &NESL_Mapper4RomRead;
    g_test.mapper.action.rom_write = &NESL_Mapper4RomWrite;

    return NESL_Mapper4Init(&g_test.mapper);
}

static int NESL_TestMapper4Init(void)
{
    int result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    header.rom.program = 2;
    header.rom.character = 2;

    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((g_test.mapper.ram.program == 0)
            && (g_test.mapper.rom.character[0] == 0)
            && (g_test.mapper.rom.character[1] == 1024)
            && (g_test.mapper.rom.character[2] == 0)
            && (g_test.mapper.rom.character[3] == 1024)
            && (g_test.mapper.rom.character[4] == 0)
            && (g_test.mapper.rom.character[5] == 0)
            && (g_test.mapper.rom.character[6] == 0)
            && (g_test.mapper.rom.character[7] == 0)
            && (g_test.mapper.rom.program[0] == 0)
            && (g_test.mapper.rom.program[1] == 0)
            && (g_test.mapper.rom.program[2] == (header.rom.program * 16 * 1024) - (2 * 8 * 1024))
            && (g_test.mapper.rom.program[3] == (header.rom.program * 16 * 1024) - (1 * 8 * 1024))
            && (g_test.mapper.mirror == NESL_MIRROR_VERTICAL)
            && (g_test.mapper.context != NULL)
            && (g_test.mapper.action.interrupt == &NESL_Mapper4Interrupt)
            && (g_test.mapper.action.ram_read == &NESL_Mapper4RamRead)
            && (g_test.mapper.action.ram_write == &NESL_Mapper4RamWrite)
            && (g_test.mapper.action.reset == &NESL_Mapper4Reset)
            && (g_test.mapper.action.rom_read == &NESL_Mapper4RomRead)
            && (g_test.mapper.action.rom_write == &NESL_Mapper4RomWrite))) {
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
            && (g_test.mapper.rom.character[1] == 1024)
            && (g_test.mapper.rom.character[2] == 0)
            && (g_test.mapper.rom.character[3] == 1024)
            && (g_test.mapper.rom.character[4] == 0)
            && (g_test.mapper.rom.character[5] == 0)
            && (g_test.mapper.rom.character[6] == 0)
            && (g_test.mapper.rom.character[7] == 0)
            && (g_test.mapper.rom.program[0] == 0)
            && (g_test.mapper.rom.program[1] == 0)
            && (g_test.mapper.rom.program[2] == (header.rom.program * 16 * 1024) - (2 * 8 * 1024))
            && (g_test.mapper.rom.program[3] == (header.rom.program * 16 * 1024) - (1 * 8 * 1024))
            && (g_test.mapper.mirror == NESL_MIRROR_VERTICAL)
            && (g_test.mapper.context != NULL)
            && (g_test.mapper.action.interrupt == &NESL_Mapper4Interrupt)
            && (g_test.mapper.action.ram_read == &NESL_Mapper4RamRead)
            && (g_test.mapper.action.ram_write == &NESL_Mapper4RamWrite)
            && (g_test.mapper.action.reset == &NESL_Mapper4Reset)
            && (g_test.mapper.action.rom_read == &NESL_Mapper4RomRead)
            && (g_test.mapper.action.rom_write == &NESL_Mapper4RomWrite))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestMapper4Interrupt(void)
{
    int result = NESL_SUCCESS;
    nesl_mapper_4_context_t *context;
    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
        goto exit;
    }

    context = (nesl_mapper_4_context_t *)g_test.mapper.context;
    context->interrupt.count = 0;
    context->interrupt.enable = false;
    context->interrupt.latch = 10;

    if(NESL_ASSERT(NESL_Mapper4Interrupt(&g_test.mapper) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT(context->interrupt.count == context->interrupt.latch)) {
        result = NESL_FAILURE;
        goto exit;
    }

    context->interrupt.count = 1;
    context->interrupt.enable = false;

    if(NESL_ASSERT(NESL_Mapper4Interrupt(&g_test.mapper) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT(context->interrupt.count == 0)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT(NESL_Mapper4Interrupt(&g_test.mapper) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT((context->interrupt.count == context->interrupt.latch)
            && (g_test.int_type == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

    context->interrupt.count = 1;
    context->interrupt.enable = true;

    if(NESL_ASSERT(NESL_Mapper4Interrupt(&g_test.mapper) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT(NESL_Mapper4Interrupt(&g_test.mapper) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT((context->interrupt.count == context->interrupt.latch)
            && (g_test.int_type == NESL_INTERRUPT_MASKABLE))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestMapper4RamRead(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x6000 ... 0x7FFF:

                for(int type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

                    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    if(type == NESL_BANK_RAM_PROGRAM) {
                        g_test.cartridge.ram.program[address & 0x1FFF] = data;

                        if(NESL_ASSERT((NESL_Mapper4RamRead(&g_test.mapper, type, address) == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }

                        ((nesl_mapper_4_context_t *)g_test.mapper.context)->protect.ram_enable = false;

                        if(NESL_ASSERT((NESL_Mapper4RamRead(&g_test.mapper, type, address) == 0)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }

                        ((nesl_mapper_4_context_t *)g_test.mapper.context)->protect.ram_enable = true;

                        if(NESL_ASSERT((NESL_Mapper4RamRead(&g_test.mapper, type, address) == data)
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

static int NESL_TestMapper4RamWrite(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x6000 ... 0x7FFF:

                for(int type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

                    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    NESL_Mapper4RamWrite(&g_test.mapper, type, address, data);

                    if(type == NESL_BANK_RAM_PROGRAM) {

                        if(NESL_ASSERT((g_test.data == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }

                        ((nesl_mapper_4_context_t *)g_test.mapper.context)->protect.ram_enable = false;
                        NESL_Mapper4RamWrite(&g_test.mapper, type, address, data + 1);

                        if(NESL_ASSERT((g_test.data == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x1FFF)))) {
                            result = NESL_FAILURE;
                            goto exit;
                        }

                        ((nesl_mapper_4_context_t *)g_test.mapper.context)->protect.ram_enable = true;
                        ((nesl_mapper_4_context_t *)g_test.mapper.context)->protect.ram_read_only = true;
                        NESL_Mapper4RamWrite(&g_test.mapper, type, address, data + 1);

                        if(NESL_ASSERT((g_test.data == data)
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

static int NESL_TestMapper4Reset(void)
{
    int result = NESL_SUCCESS;
    nesl_mapper_4_context_t *context;
    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_Mapper4Reset(&g_test.mapper) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    context = (nesl_mapper_4_context_t *)g_test.mapper.context;

    if(NESL_ASSERT((context->protect.ram_enable == true)
            && (context->protect.ram_read_only == false)
            && (g_test.mapper.mirror == (context->mirror.mode ? NESL_MIRROR_HORIZONTAL : NESL_MIRROR_VERTICAL))
            && (g_test.mapper.rom.character[0] == context->bank.index[0] * 1 * 1024)
            && (g_test.mapper.rom.character[1] == g_test.mapper.rom.character[0] + 1024)
            && (g_test.mapper.rom.character[2] == context->bank.index[1] * 1 * 1024)
            && (g_test.mapper.rom.character[3] == g_test.mapper.rom.character[2] + 1024)
            && (g_test.mapper.rom.character[4] == context->bank.index[2] * 1 * 1024)
            && (g_test.mapper.rom.character[5] == context->bank.index[3] * 1 * 1024)
            && (g_test.mapper.rom.character[6] == context->bank.index[4] * 1 * 1024)
            && (g_test.mapper.rom.character[7] == context->bank.index[5] * 1 * 1024)
            && (g_test.mapper.rom.program[0] == context->bank.index[6] * 8 * 1024)
            && (g_test.mapper.rom.program[1] == context->bank.index[7] * 8 * 1024)
            && (g_test.mapper.rom.program[2] == (g_test.mapper.cartridge.header->rom.program * 16 * 1024) - (2 * 8 * 1024))
            && (g_test.mapper.rom.program[3] == (g_test.mapper.cartridge.header->rom.program * 16 * 1024) - (1 * 8 * 1024)))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestMapper4RomRead(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x0000 ... 0x1FFF:

                for(int type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

                    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    for(uint8_t index = 0; index < 8; ++index) {
                        g_test.mapper.rom.character[index] = 0;
                    }

                    if(type == NESL_BANK_ROM_CHARACTER) {
                        g_test.cartridge.rom.character[address & 0x03FF] = data;

                        if(NESL_ASSERT((NESL_Mapper4RomRead(&g_test.mapper, type, address) == data)
                                && (g_test.type == type)
                                && (g_test.address == (address & 0x03FF)))) {
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
                for(int type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

                    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    for(uint8_t index = 0; index < 4; ++index) {
                        g_test.mapper.rom.program[index] = 0;
                    }

                    if(type == NESL_BANK_ROM_PROGRAM) {
                        g_test.cartridge.rom.program[address & 0x1FFF] = data;

                        if(NESL_ASSERT((NESL_Mapper4RomRead(&g_test.mapper, type, address) == data)
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

static int NESL_TestMapper4RomWrite(void)
{
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x8000; address <= 0xFFFF; ++address) {
        nesl_mapper_4_context_t *context;
        nesl_cartridge_header_t header = { .rom.program = 2, .rom.character = 2 };

        if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
            goto exit;
        }

        context = (nesl_mapper_4_context_t *)g_test.mapper.context;

        switch(address) {
            case 0x8000 ... 0x9FFF:

                for(uint8_t bank = 0; bank < 8; ++bank) {
                    nesl_mapper_4_select_t select = { .bank = bank };

                    if(!(address % 2)) {
                        select.program = 1;
                        select.character = 1;
                        NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, select.raw);

                        if(NESL_ASSERT(context->select.raw == select.raw)) {
                            result = NESL_FAILURE;
                            goto exit;
                        }
                    } else {

                        switch(bank) {
                            case 0:
                                select.character = 0;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[0] == (context->bank.index[0] * 1 * 1024)
                                        && (g_test.mapper.rom.character[1] == (g_test.mapper.rom.character[0] + 1024))))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }

                                select.character = 1;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[4] == (context->bank.index[0] * 1 * 1024)
                                        && (g_test.mapper.rom.character[5] == (g_test.mapper.rom.character[4] + 1024))))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }
                                break;
                            case 1:
                                select.character = 0;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[2] == (context->bank.index[1] * 1 * 1024)
                                        && (g_test.mapper.rom.character[3] == (g_test.mapper.rom.character[2] + 1024))))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }

                                select.character = 1;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[6] == (context->bank.index[1] * 1 * 1024)
                                        && (g_test.mapper.rom.character[7] == (g_test.mapper.rom.character[6] + 1024))))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }
                                break;
                            case 2:
                                select.character = 0;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[4] == (context->bank.index[2] * 1 * 1024)))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }

                                select.character = 1;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[0] == (context->bank.index[2] * 1 * 1024)))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }
                                break;
                            case 3:
                                select.character = 0;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[5] == (context->bank.index[2] * 1 * 1024)))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }

                                select.character = 1;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[1] == (context->bank.index[2] * 1 * 1024)))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }
                                break;
                            case 4:
                                select.character = 0;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[6] == (context->bank.index[2] * 1 * 1024)))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }

                                select.character = 1;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[2] == (context->bank.index[2] * 1 * 1024)))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }
                                break;
                            case 5:
                                select.character = 0;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[7] == (context->bank.index[2] * 1 * 1024)))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }

                                select.character = 1;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.character[3] == (context->bank.index[2] * 1 * 1024)))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }
                                break;
                            case 6:
                                select.program = 0;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.program[0] == context->bank.index[6] * 8 * 1024))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }

                                select.program = 1;
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.program[2] == context->bank.index[6] * 8 * 1024))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }
                                break;
                            case 7:
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address - 1, select.raw);
                                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 10);

                                if(NESL_ASSERT((context->bank.data == 10)
                                        && (context->bank.index[bank] == 10)
                                        && (g_test.mapper.rom.program[1] == context->bank.index[7] * 8 * 1024))) {
                                    result = NESL_FAILURE;
                                    goto exit;
                                }
                                break;
                            default:
                                break;
                        }
                    }
                }
                break;
            case 0xA000 ... 0xBFFF:

                if(!(address % 2)) {
                    nesl_mapper_4_mirror_t mirror = {};

                    mirror.mode = 1;
                    NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, mirror.raw);

                    if(NESL_ASSERT(g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL)) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    mirror.mode = 0;
                    NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, mirror.raw);

                    if(NESL_ASSERT(g_test.mapper.mirror == NESL_MIRROR_VERTICAL)) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                } else {
                    nesl_mapper_4_protect_t protect = {};

                    protect.ram_enable = 1;
                    protect.ram_read_only = 0;
                    NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, protect.raw);

                    if(NESL_ASSERT(context->protect.raw == protect.raw)) {
                        result = NESL_FAILURE;
                        goto exit;
                    }

                    protect.ram_enable = 0;
                    protect.ram_read_only = 1;
                    NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, protect.raw);

                    if(NESL_ASSERT(context->protect.raw == protect.raw)) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;
            case 0xC000 ... 0xDFFF:

                if(!(address % 2)) {
                    NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 0xEF);

                    if(NESL_ASSERT(context->interrupt.latch == 0xEF)) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                } else {
                    NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 0);

                    if(NESL_ASSERT(context->interrupt.count == 0)) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
                }
                break;
            case 0xE000 ... 0xFFFF:
                NESL_Mapper4RomWrite(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, 0);

                if(NESL_ASSERT(context->interrupt.enable == (address % 2))) {
                    result = NESL_FAILURE;
                    goto exit;
                }
                break;
            default:
                break;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestMapper4Uninit(void)
{
    int result = NESL_SUCCESS;
    nesl_cartridge_header_t header = {};

    if((result = NESL_TestInit(&header)) == NESL_FAILURE) {
        goto exit;
    }

    NESL_Mapper4Uninit(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper.context == NULL)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static const NESL_Test TEST[] = {
    NESL_TestMapper4Init,
    NESL_TestMapper4Interrupt,
    NESL_TestMapper4RamRead,
    NESL_TestMapper4RamWrite,
    NESL_TestMapper4Reset,
    NESL_TestMapper4RomRead,
    NESL_TestMapper4RomWrite,
    NESL_TestMapper4Uninit,
    };

int main(void)
{
    int result = NESL_SUCCESS;

    for(int index = 0; index < NESL_TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    return result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
