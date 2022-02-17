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

#include "../../include/system/processor.h"
#include "../include/common.h"

typedef struct {
    int type;
    int mode;
    uint8_t cycles;
} nesl_instruction_t;

typedef struct {
    nesl_processor_t processor;

    struct {
        int type;
        uint16_t address;
        uint8_t ram[64 * 1024];
        uint8_t oam[64 * 4];
    } bus;
} nesl_test_t;

static nesl_test_t g_test = {};

/*static const nesl_instruction_t UNSUPPORTED[] = {
    { 0x02, NESL_ADDRESS_IMPLIED, 2 },
    { 0x03, NESL_ADDRESS_INDIRECT_X, 8 },
    { 0x04, NESL_ADDRESS_ZEROPAGE, 3 },
    { 0x07, NESL_ADDRESS_ZEROPAGE, 5 },
    { 0x0B, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0x0C, NESL_ADDRESS_ABSOLUTE, 4 },
    { 0x0F, NESL_ADDRESS_ABSOLUTE, 6 },
    { 0x12, NESL_ADDRESS_IMPLIED, 2 },
    { 0x13, NESL_ADDRESS_INDIRECT_Y, 8 },
    { 0x14, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { 0x17, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { 0x1A, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0x1B, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { 0x1C, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { 0x1F, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { 0x22, NESL_ADDRESS_IMPLIED, 2 },
    { 0x23, NESL_ADDRESS_INDIRECT_X, 8 },
    { 0x27, NESL_ADDRESS_ZEROPAGE, 5 },
    { 0x2B, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0x2F, NESL_ADDRESS_ABSOLUTE, 6 },
    { 0x32, NESL_ADDRESS_IMPLIED, 2 },
    { 0x33, NESL_ADDRESS_INDIRECT_Y, 8 },
    { 0x34, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { 0x37, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { 0x3A, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0x3B, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { 0x3C, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { 0x3F, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { 0x42, NESL_ADDRESS_IMPLIED, 2 },
    { 0x43, NESL_ADDRESS_INDIRECT_X, 8 },
    { 0x44, NESL_ADDRESS_ZEROPAGE, 3 },
    { 0x47, NESL_ADDRESS_ZEROPAGE, 5 },
    { 0x4B, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0x4F, NESL_ADDRESS_ABSOLUTE, 6 },
    { 0x52, NESL_ADDRESS_IMPLIED, 2 },
    { 0x53, NESL_ADDRESS_INDIRECT_Y, 8 },
    { 0x54, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { 0x57, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { 0x5A, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0x5B, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { 0x5C, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { 0x5F, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { 0x62, NESL_ADDRESS_IMPLIED, 2 },
    { 0x63, NESL_ADDRESS_INDIRECT_X, 8 },
    { 0x64, NESL_ADDRESS_ZEROPAGE, 3 },
    { 0x67, NESL_ADDRESS_ZEROPAGE, 5 },
    { 0x6B, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0x6F, NESL_ADDRESS_ABSOLUTE, 6 },
    { 0x72, NESL_ADDRESS_IMPLIED, 2 },
    { 0x73, NESL_ADDRESS_INDIRECT_Y, 8 },
    { 0x74, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { 0x77, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { 0x7A, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0x7B, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { 0x7C, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { 0x7F, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { 0x82, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0x83, NESL_ADDRESS_INDIRECT_X, 6 },
    { 0x87, NESL_ADDRESS_ZEROPAGE, 3 },
    { 0x89, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0x8B, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0x8F, NESL_ADDRESS_ABSOLUTE, 4 },
    { 0x92, NESL_ADDRESS_IMPLIED, 2 },
    { 0x93, NESL_ADDRESS_INDIRECT_Y, 6 },
    { 0x97, NESL_ADDRESS_ZEROPAGE_Y, 4 },
    { 0x9B, NESL_ADDRESS_ABSOLUTE_Y, 5 },
    { 0x9C, NESL_ADDRESS_ABSOLUTE_X, 5 },
    { 0x9E, NESL_ADDRESS_ABSOLUTE_Y, 5 },
    { 0x9F, NESL_ADDRESS_ABSOLUTE_Y, 5 },
    { 0xA3, NESL_ADDRESS_INDIRECT_X, 6 },
    { 0xA7, NESL_ADDRESS_ZEROPAGE, 3 },
    { 0xAB, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0xAF, NESL_ADDRESS_ABSOLUTE, 4 },
    { 0xB2, NESL_ADDRESS_IMPLIED, 2 },
    { 0xB3, NESL_ADDRESS_INDIRECT_Y, 5 },
    { 0xB7, NESL_ADDRESS_ZEROPAGE_Y, 4 },
    { 0xBB, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { 0xBF, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { 0xC2, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0xC3, NESL_ADDRESS_INDIRECT_X, 8 },
    { 0xC7, NESL_ADDRESS_ZEROPAGE, 5 },
    { 0xCB, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0xCF, NESL_ADDRESS_ABSOLUTE, 6 },
    { 0xD2, NESL_ADDRESS_IMPLIED, 2 },
    { 0xD3, NESL_ADDRESS_INDIRECT_Y, 8 },
    { 0xD4, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { 0xD7, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { 0xDA, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0xDB, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { 0xDC, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { 0xDF, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { 0xE2, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0xE3, NESL_ADDRESS_INDIRECT_X, 8 },
    { 0xE7, NESL_ADDRESS_ZEROPAGE, 5 },
    { 0xEB, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0xEF, NESL_ADDRESS_ABSOLUTE, 6 },
    { 0xF2, NESL_ADDRESS_IMPLIED, 2 },
    { 0xF3, NESL_ADDRESS_INDIRECT_Y, 8 },
    { 0xF4, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { 0xF7, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { 0xFA, NESL_ADDRESS_IMMEDIATE, 2 },
    { 0xFB, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { 0xFC, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { 0xFF, NESL_ADDRESS_ABSOLUTE_X, 7 },
    };*/

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uint8_t nesl_bus_read(int type, uint16_t address)
{
    int result = 0;

    g_test.bus.type = type;
    g_test.bus.address = address;

    switch(type) {
        case NESL_BUS_PROCESSOR:
            result = g_test.bus.ram[g_test.bus.address];
            break;
        case NESL_BUS_VIDEO_OAM:
            result = g_test.bus.oam[g_test.bus.address];
            break;
        default:
            break;
    }

    return result;
}

void nesl_bus_write(int type, uint16_t address, uint8_t data)
{
    g_test.bus.type = type;
    g_test.bus.address = address;

    switch(type) {
        case NESL_BUS_PROCESSOR:
            g_test.bus.ram[g_test.bus.address] = data;
            break;
        case NESL_BUS_VIDEO_OAM:
            g_test.bus.oam[g_test.bus.address] = data;
            break;
        default:
            break;
    }
}

/*static int nesl_test_initialize(uint16_t address, bool initialize)
{
    int result = NESL_SUCCESS;

    memset(&g_test.processor, 0, sizeof(g_test.processor));

    if(initialize) {
        nesl_bus_write(NESL_BUS_PROCESSOR, 0xFFFC, address);
        nesl_bus_write(NESL_BUS_PROCESSOR, 0xFFFD, address >> 8);

        if((result = nesl_processor_initialize(&g_test.processor)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    return result;
}*/

/* TODO */

static const nesl_test TEST[] = {

    /* TODO */

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
