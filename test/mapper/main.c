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

#include "../../include/system/NESL_mapper.h"
#include "../include/NESL_common.h"

typedef struct {
    nesl_mapper_t mapper;
    int type;
    uint16_t address;
    uint8_t data;
    bool interrupt;
    bool reset;

    struct {
        const nesl_header_t *header;
        const void *data;
        int length;
        bool initialized;
        int status;
    } cartridge;

    struct {
        bool initialized;
        int status;
    } mapper_0;

    struct {
        bool initialized;
        bool reset;
        int status;
    } mapper_1;

    struct {
        bool initialized;
        bool reset;
        int status;
    } mapper_2;

    struct {
        bool initialized;
        bool reset;
        int status;
    } mapper_3;

    struct {
        bool initialized;
        bool interrupt;
        bool reset;
        int status;
    } mapper_4;

    struct {
        bool initialized;
        bool reset;
        int status;
    } mapper_30;

    struct {
        bool initialized;
        bool reset;
        int status;
    } mapper_66;
} nesl_test_t;

static nesl_test_t g_test = {};

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

int NESL_CartridgeInit(nesl_cartridge_t *cartridge, const void *data, int length)
{
    g_test.cartridge.data = data;
    g_test.cartridge.length = length;
    g_test.cartridge.initialized = (g_test.cartridge.status == NESL_SUCCESS);

    return g_test.cartridge.status;
}

void NESL_CartridgeUninit(nesl_cartridge_t *cartridge)
{
    g_test.cartridge.initialized = false;
}

int NESL_Mapper0Init(nesl_mapper_t *mapper)
{
    g_test.mapper_0.initialized = (g_test.mapper_0.status == NESL_SUCCESS);

    return g_test.mapper_0.status;
}

uint8_t NESL_Mapper0RamRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper0RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

uint8_t NESL_Mapper0RomRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

int NESL_Mapper1Init(nesl_mapper_t *mapper)
{
    g_test.mapper_1.initialized = (g_test.mapper_1.status == NESL_SUCCESS);

    return g_test.mapper_0.status;
}

uint8_t NESL_Mapper1RamRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper1RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int NESL_Mapper1Reset(nesl_mapper_t *mapper)
{
    g_test.mapper_1.reset = true;

    return NESL_SUCCESS;
}

uint8_t NESL_Mapper1RomRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper1RomWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void NESL_Mapper1Uninit(nesl_mapper_t *mapper)
{
    g_test.mapper_1.initialized = false;
}

int NESL_Mapper2Init(nesl_mapper_t *mapper)
{
    g_test.mapper_2.initialized = (g_test.mapper_2.status == NESL_SUCCESS);

    return g_test.mapper_2.status;
}

uint8_t NESL_Mapper2RamRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper2RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int NESL_Mapper2Reset(nesl_mapper_t *mapper)
{
    g_test.mapper_2.reset = true;

    return NESL_SUCCESS;
}

uint8_t NESL_Mapper2RomRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper2RomWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void NESL_Mapper2Uninit(nesl_mapper_t *mapper)
{
    g_test.mapper_2.initialized = false;
}

int NESL_Mapper3Init(nesl_mapper_t *mapper)
{
    g_test.mapper_3.initialized = (g_test.mapper_3.status == NESL_SUCCESS);

    return g_test.mapper_3.status;
}

uint8_t NESL_Mapper3RamRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper3RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int NESL_Mapper3Reset(nesl_mapper_t *mapper)
{
    g_test.mapper_3.reset = true;

    return NESL_SUCCESS;
}

uint8_t NESL_Mapper3RomRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper3RomWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void NESL_Mapper3Uninit(nesl_mapper_t *mapper)
{
    g_test.mapper_3.initialized = false;
}

int NESL_Mapper30Init(nesl_mapper_t *mapper)
{
    g_test.mapper_30.initialized = (g_test.mapper_30.status == NESL_SUCCESS);

    return g_test.mapper_30.status;
}

uint8_t NESL_Mapper30RamRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper30RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int NESL_Mapper30Reset(nesl_mapper_t *mapper)
{
    g_test.mapper_30.reset = true;

    return NESL_SUCCESS;
}

uint8_t NESL_Mapper30RomRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper30RomWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void NESL_Mapper30Uninit(nesl_mapper_t *mapper)
{
    g_test.mapper_30.initialized = false;
}

int NESL_Mapper4Init(nesl_mapper_t *mapper)
{
    g_test.mapper_4.initialized = (g_test.mapper_4.status == NESL_SUCCESS);

    return g_test.mapper_4.status;
}

int NESL_Mapper4Interrupt(nesl_mapper_t *mapper)
{
    g_test.mapper_4.interrupt = true;

    return NESL_SUCCESS;
}

uint8_t NESL_Mapper4RamRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper4RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int NESL_Mapper4Reset(nesl_mapper_t *mapper)
{
    g_test.mapper_4.reset = true;

    return NESL_SUCCESS;
}

uint8_t NESL_Mapper4RomRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper4RomWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void NESL_Mapper4Uninit(nesl_mapper_t *mapper)
{
    g_test.mapper_4.initialized = false;
}

int NESL_Mapper66Init(nesl_mapper_t *mapper)
{
    g_test.mapper_66.initialized = (g_test.mapper_66.status == NESL_SUCCESS);

    return g_test.mapper_66.status;
}

uint8_t NESL_Mapper66RamRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper66RamWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

int NESL_Mapper66Reset(nesl_mapper_t *mapper)
{
    g_test.mapper_66.reset = true;

    return NESL_SUCCESS;
}

uint8_t NESL_Mapper66RomRead(nesl_mapper_t *mapper, int type, uint16_t address)
{
    return 0;
}

void NESL_Mapper66RomWrite(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    return;
}

void NESL_Mapper66Uninit(nesl_mapper_t *mapper)
{
    g_test.mapper_66.initialized = false;
}

int NESL_SetError(const char *file, const char *function, int line, const char *format, ...)
{
    return NESL_FAILURE;
}

static int NESL_TestInterruptHandler(nesl_mapper_t *mapper)
{
    g_test.interrupt = true;

    return NESL_SUCCESS;
}

static uint8_t NESL_TestReadHandler(nesl_mapper_t *mapper, int type, uint16_t address)
{
    g_test.type = type;
    g_test.address = address;

    return g_test.data;
}

static int NESL_TestResetHandler(nesl_mapper_t *mapper)
{
    g_test.reset = true;

    return NESL_SUCCESS;
}

static void NESL_TestWriteHandler(nesl_mapper_t *mapper, int type, uint16_t address, uint8_t data)
{
    g_test.type = type;
    g_test.address = address;
    g_test.data = data;
}

static void NESL_TestInit(const nesl_header_t *header)
{
    memset(&g_test, 0, sizeof(g_test));
    g_test.cartridge.header = header;
    g_test.mapper.cartridge.header = g_test.cartridge.header;
    g_test.mapper.interrupt = &NESL_TestInterruptHandler;
    g_test.mapper.ram_read = &NESL_TestReadHandler;
    g_test.mapper.ram_write = &NESL_TestWriteHandler;
    g_test.mapper.reset = &NESL_TestResetHandler;
    g_test.mapper.rom_read = &NESL_TestReadHandler;
    g_test.mapper.rom_write = &NESL_TestWriteHandler;
}

static int NESL_TestMapperInit(void)
{
    int result = NESL_SUCCESS;
    nesl_header_t header = {};

    NESL_TestInit(&header);
    g_test.cartridge.status = NESL_FAILURE;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    g_test.mapper_0.status = NESL_FAILURE;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = 0xF;
    header.flag_7.type_high = 0xF;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_FAILURE)) {
        result = NESL_FAILURE;
        goto exit;
    }

    memset(&header, 0, sizeof(header));

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_0;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT((NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_0.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_0)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_1;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT((NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_1.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_1)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_2;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT((NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_2.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_2)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_3;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT((NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_3.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_3)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_30 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_30 & 0xF0) >> 4;

    if(NESL_ASSERT((NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_30.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_30)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_4;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT((NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_4.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_4)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_66 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_66 & 0xF0) >> 4;

    if(NESL_ASSERT((NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)
            && (g_test.cartridge.data == &header)
            && (g_test.cartridge.length == sizeof(header))
            && (g_test.cartridge.initialized == true)
            && (g_test.mapper_66.initialized == true)
            && (g_test.mapper.type == NESL_MAPPER_66)
            && (g_test.mapper.mirror == NESL_MIRROR_HORIZONTAL))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestMapperInterrupt(void)
{
    int result = NESL_SUCCESS;
    nesl_header_t header = {};

    NESL_TestInit(&header);
    NESL_MapperInterrupt(&g_test.mapper);

    if(NESL_ASSERT(g_test.interrupt == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_4;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperInterrupt(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_4.interrupt == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestMapperRead(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address, ++data) {

        for(int type = 0; type < NESL_BANK_MAX; ++type) {
            nesl_header_t header = {};

            NESL_TestInit(&header);
            g_test.data = data;

            if(NESL_ASSERT((NESL_MapperRead(&g_test.mapper, type, address) == data)
                    && (g_test.type == type)
                    && (g_test.address == address))) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestMapperReset(void)
{
    int result = NESL_SUCCESS;
    nesl_header_t header = {};

    NESL_TestInit(&header);
    NESL_MapperReset(&g_test.mapper);

    if(NESL_ASSERT(g_test.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_1;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperReset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_1.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_2;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperReset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_2.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_3;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperReset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_3.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_30 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_30 & 0xF0) >> 4;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperReset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_30.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_4;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperReset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_4.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_66 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_66 & 0xF0) >> 4;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperReset(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_66.reset == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestMapperUninit(void)
{
    nesl_header_t header = {};
    int result = NESL_SUCCESS;

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_1;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperUninit(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_1.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_2;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperUninit(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_2.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_3;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperUninit(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_3.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_30 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_30 & 0xF0) >> 4;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperUninit(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_30.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_4;
    header.flag_7.type_high = 0;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperUninit(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_4.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_TestInit(&header);
    header.flag_6.type_low = NESL_MAPPER_66 & 0x0F;
    header.flag_7.type_high = (NESL_MAPPER_66 & 0xF0) >> 4;

    if(NESL_ASSERT(NESL_MapperInit(&g_test.mapper, &header, sizeof(header)) == NESL_SUCCESS)) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_MapperUninit(&g_test.mapper);

    if(NESL_ASSERT(g_test.mapper_66.initialized == false)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static int NESL_TestMapperWrite(void)
{
    uint8_t data = 0;
    int result = NESL_SUCCESS;

    for(uint32_t address = 0x0000; address <= 0xFFFF; ++address) {

        for(int type = 0; type < NESL_BANK_MAX; ++type, ++data) {
            nesl_header_t header = {};

            NESL_TestInit(&header);
            NESL_MapperWrite(&g_test.mapper, type, address, data);

            if(NESL_ASSERT((g_test.data == data)
                    && (g_test.type == type)
                    && (g_test.address == address))) {
                result = NESL_FAILURE;
                goto exit;
            }
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static const NESL_Test TEST[] = {
    NESL_TestMapperInit,
    NESL_TestMapperInterrupt,
    NESL_TestMapperRead,
    NESL_TestMapperReset,
    NESL_TestMapperUninit,
    NESL_TestMapperWrite,
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
