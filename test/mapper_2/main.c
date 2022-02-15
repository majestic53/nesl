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

#include "../../include/system/mapper/mapper_2.h"
#include "../include/common.h"

typedef struct {
    nesl_mapper_t mapper;
    int type;
    uint32_t address;
    uint8_t data;

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

uint8_t nesl_cartridge_ram_read(nesl_cartridge_t *cartridge, int type, uint32_t address)
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

void nesl_cartridge_ram_write(nesl_cartridge_t *cartridge, int type, uint32_t address, uint8_t data)
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

uint8_t nesl_cartridge_rom_read(nesl_cartridge_t *cartridge, int type, uint32_t address)
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

int nesl_error_set(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

static void nesl_test_uninitialize(void)
{
    nesl_mapper_2_uninitialize(&g_test.mapper);
    g_test.mapper.interrupt = NULL;
    g_test.mapper.ram_read = NULL;
    g_test.mapper.ram_write = NULL;
    g_test.mapper.reset = NULL;
    g_test.mapper.rom_read = NULL;
    g_test.mapper.rom_write = NULL;
}

static int nesl_test_initialize(const nesl_header_t *header)
{
    nesl_test_uninitialize();
    memset(&g_test, 0, sizeof(g_test));
    g_test.mapper.cartridge.header = header;
    g_test.mapper.interrupt = NULL;
    g_test.mapper.ram_read = &nesl_mapper_2_ram_read;
    g_test.mapper.ram_write = &nesl_mapper_2_ram_write;
    g_test.mapper.reset = &nesl_mapper_2_reset;
    g_test.mapper.rom_read = &nesl_mapper_2_rom_read;
    g_test.mapper.rom_write = &nesl_mapper_2_rom_write;

    return nesl_mapper_2_initialize(&g_test.mapper);
}

static int nesl_test_mapper_2_initialize(void)
{
    int result = NESL_SUCCESS;
    nesl_header_t header = {};

    header.rom.program = 2;

    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((g_test.mapper.ram.program == 0)
            && (g_test.mapper.rom.character[0] == 0)
            && (g_test.mapper.rom.program[0] == 0)
            && (g_test.mapper.rom.program[1] == 16 * 1024)
            && (g_test.mapper.interrupt == NULL)
            && (g_test.mapper.ram_read == &nesl_mapper_2_ram_read)
            && (g_test.mapper.ram_write == &nesl_mapper_2_ram_write)
            && (g_test.mapper.reset == &nesl_mapper_2_reset)
            && (g_test.mapper.rom_read == &nesl_mapper_2_rom_read)
            && (g_test.mapper.rom_write == &nesl_mapper_2_rom_write)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL)
            && (g_test.mapper.context != NULL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    header.rom.program = 4;

    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((g_test.mapper.ram.program == 0)
            && (g_test.mapper.rom.character[0] == 0)
            && (g_test.mapper.rom.program[0] == 0)
            && (g_test.mapper.rom.program[1] == 3 * 16 * 1024)
            && (g_test.mapper.interrupt == NULL)
            && (g_test.mapper.ram_read == &nesl_mapper_2_ram_read)
            && (g_test.mapper.ram_write == &nesl_mapper_2_ram_write)
            && (g_test.mapper.reset == &nesl_mapper_2_reset)
            && (g_test.mapper.rom_read == &nesl_mapper_2_rom_read)
            && (g_test.mapper.rom_write == &nesl_mapper_2_rom_write)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL)
            && (g_test.mapper.context != NULL))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_mapper_2_ram_read(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x6000 ... 0x7FFF:

                for(int type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_header_t header = { .rom.program = 1, .rom.character = 2 };

                    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    if(type == NESL_BANK_RAM_PROGRAM) {
                        g_test.cartridge.ram.program[address & 0x1FFF] = data;

                        if(NESL_ASSERT((nesl_mapper_2_ram_read(&g_test.mapper, type, address) == data)
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

static int nesl_test_mapper_2_ram_write(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x6000 ... 0x7FFF:

                for(int type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_header_t header = { .rom.program = 1, .rom.character = 2 };

                    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    nesl_mapper_2_ram_write(&g_test.mapper, type, address, data);

                    if(type == NESL_BANK_RAM_PROGRAM) {

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

static int nesl_test_mapper_2_reset(void)
{
    int result = NESL_SUCCESS;
    nesl_header_t header = { .rom.program = 2, .rom.character = 2 };

    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(nesl_mapper_2_reset(&g_test.mapper) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if(NESL_ASSERT((g_test.mapper.ram.program == 0)
            && (g_test.mapper.rom.character[0] == 0)
            && (g_test.mapper.rom.program[0] == (((nesl_mapper_2_context_t *)g_test.mapper.context)->program.bank * 16 * 1024))
            && (g_test.mapper.rom.program[1] == (g_test.mapper.cartridge.header->rom.program * 16 * 1024) - (1 * 16 * 1024)))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int nesl_test_mapper_2_rom_read(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        switch(address) {
            case 0x0000 ... 0x1FFF:

                for(int type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_header_t header = { .rom.program = 2, .rom.character = 1 };

                    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    if(type == NESL_BANK_ROM_CHARACTER) {
                        g_test.cartridge.rom.character[address & 0x1FFF] = data;

                        if(NESL_ASSERT((nesl_mapper_2_rom_read(&g_test.mapper, type, address) == data)
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

                for(int type = 0; type < NESL_BANK_MAX; ++type) {
                    nesl_header_t header = { .rom.program = 2, .rom.character = 1 };

                    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
                        goto exit;
                    }

                    if(type == NESL_BANK_ROM_PROGRAM) {
                        g_test.cartridge.rom.program[address & 0x7FFF] = data;

                        if(NESL_ASSERT((nesl_mapper_2_rom_read(&g_test.mapper, type, address) == data)
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

static int nesl_test_mapper_2_rom_write(void)
{
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address) {
        nesl_header_t header = { .rom.program = 4, .rom.character = 1 };

        switch(address) {
            case 0x8000 ... 0xFFFF:

                if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
                    goto exit;
                }

                for(uint8_t bank = 0; bank < 4; ++bank) {
                    nesl_mapper_2_program_t program = { .bank = bank};

                    nesl_mapper_2_rom_write(&g_test.mapper, NESL_BANK_ROM_PROGRAM, address, program.raw);

                    if(NESL_ASSERT((g_test.mapper.rom.character[0] == 0)
                            && (g_test.mapper.rom.program[1] == (3 * 16 * 1024))
                            && (g_test.mapper.rom.program[0] == (bank * 16 * 1024)))) {
                        result = NESL_FAILURE;
                        goto exit;
                    }
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

static int nesl_test_mapper_2_uninitialize(void)
{
    int result = NESL_SUCCESS;
    nesl_header_t header = {};

    if((result = nesl_test_initialize(&header)) == NESL_FAILURE) {
        goto exit;
    }

    nesl_mapper_2_uninitialize(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper.context == NULL)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static const nesl_test TEST[] = {
    nesl_test_mapper_2_initialize,
    nesl_test_mapper_2_ram_read,
    nesl_test_mapper_2_ram_write,
    nesl_test_mapper_2_reset,
    nesl_test_mapper_2_rom_read,
    nesl_test_mapper_2_rom_write,
    nesl_test_mapper_2_uninitialize,
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
