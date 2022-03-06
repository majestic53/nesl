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
 * @brief Test application for processor subsystem.
 */

#include "../../include/system/NESL_processor.h"
#include "../include/NESL_common.h"

/**
 * @struct nesl_instruction_t
 * @brief Processor instruction data.
 */
typedef struct {
    nesl_instruction_e type;    /*< Instruction type */
    nesl_operand_e mode;        /*< Instruction address mode */
    uint8_t cycles;             /*< Instruction cycles */
} nesl_instruction_t;

/**
 * @struct nesl_test_t
 * @brief Contains the test contexts.
 */
typedef struct {
    nesl_processor_t processor; /*< Processor context */

    struct {
        nesl_bus_e type;        /*< Bus type */
        uint16_t address;       /*< Bus address */
        uint8_t ram[64 * 1024]; /*< Bus RAM */
        uint8_t oam[64 * 4];    /*< Bus OAM RAM */
    } bus;
} nesl_test_t;

static nesl_test_t g_test = {}; /*< Test context */

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

uint8_t NESL_BusRead(nesl_bus_e type, uint16_t address)
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

void NESL_BusWrite(nesl_bus_e type, uint16_t address, uint8_t data)
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

static nesl_error_e NESL_TestInit(uint16_t address, bool initialize)
{
    nesl_error_e result = NESL_SUCCESS;

    memset(&g_test.processor, 0, sizeof(g_test.processor));

    if(initialize) {
        NESL_BusWrite(NESL_BUS_PROCESSOR, 0xFFFC, address);
        NESL_BusWrite(NESL_BUS_PROCESSOR, 0xFFFD, address >> 8);

        if((result = NESL_ProcessorInit(&g_test.processor)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    return result;
}

static nesl_error_e NESL_TestInstruction(uint16_t address, uint8_t opcode, nesl_operand_e mode, uint16_t effective, uint16_t indirect, uint8_t data)
{
    nesl_error_e result = NESL_SUCCESS;

    g_test.processor.cycle = 0;
    NESL_BusWrite(NESL_BUS_PROCESSOR, address, opcode);

    switch(mode) {
        case NESL_OPERAND_ABSOLUTE:
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 1, effective);
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 2, effective >> 8);
            NESL_BusWrite(NESL_BUS_PROCESSOR, effective, data);
            break;
        case NESL_OPERAND_ABSOLUTE_X:
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 1, effective);
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 2, effective >> 8);
            effective += g_test.processor.state.index.x.low;
            NESL_BusWrite(NESL_BUS_PROCESSOR, effective, data);
            break;
        case NESL_OPERAND_ABSOLUTE_Y:
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 1, effective);
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 2, effective >> 8);
            effective += g_test.processor.state.index.y.low;
            NESL_BusWrite(NESL_BUS_PROCESSOR, effective, data);
            break;
        case NESL_OPERAND_ACCUMULATOR:
            g_test.processor.state.accumulator.low = data;
            break;
        case NESL_OPERAND_IMPLIED:
            break;
        case NESL_OPERAND_IMMEDIATE:
        case NESL_OPERAND_RELATIVE:
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 1, data);
            break;
        case NESL_OPERAND_INDIRECT:
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 1, indirect);
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 2, indirect >> 8);

            if((indirect & 0xFF) == 0xFF) {
                NESL_BusWrite(NESL_BUS_PROCESSOR, indirect, effective);
                NESL_BusWrite(NESL_BUS_PROCESSOR, indirect & 0xFF00, effective >> 8);
            } else {
                NESL_BusWrite(NESL_BUS_PROCESSOR, indirect, effective);
                NESL_BusWrite(NESL_BUS_PROCESSOR, indirect + 1, effective >> 8);
            }

            NESL_BusWrite(NESL_BUS_PROCESSOR, effective, data);
            break;
        case NESL_OPERAND_INDIRECT_X:
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 1, indirect);
            indirect += g_test.processor.state.index.x.low;
            NESL_BusWrite(NESL_BUS_PROCESSOR, indirect & 0xFF, effective);
            NESL_BusWrite(NESL_BUS_PROCESSOR, (indirect + 1) & 0xFF, effective >> 8);
            NESL_BusWrite(NESL_BUS_PROCESSOR, effective, data);
            break;
        case NESL_OPERAND_INDIRECT_Y:
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 1, indirect);
            NESL_BusWrite(NESL_BUS_PROCESSOR, indirect & 0xFF, effective);
            NESL_BusWrite(NESL_BUS_PROCESSOR, (indirect + 1) & 0xFF, effective >> 8);
            effective += g_test.processor.state.index.y.low;
            NESL_BusWrite(NESL_BUS_PROCESSOR, effective, data);
            break;
        case NESL_OPERAND_ZEROPAGE:
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 1, effective);
            NESL_BusWrite(NESL_BUS_PROCESSOR, effective & 0xFF, data);
            break;
        case NESL_OPERAND_ZEROPAGE_X:
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 1, effective);
            effective += g_test.processor.state.index.x.low;
            NESL_BusWrite(NESL_BUS_PROCESSOR, effective & 0xFF, data);
            break;
        case NESL_OPERAND_ZEROPAGE_Y:
            NESL_BusWrite(NESL_BUS_PROCESSOR, address + 1, effective);
            effective += g_test.processor.state.index.y.low;
            NESL_BusWrite(NESL_BUS_PROCESSOR, effective & 0xFF, data);
            break;
        default:
            result = NESL_FAILURE;
            goto exit;
    }

    NESL_ProcessorCycle(&g_test.processor, 0);

exit:
    return result;
}

static bool NESL_TestValidate(uint8_t accumulator, uint8_t index_x, uint8_t index_y, uint16_t program_counter, uint8_t stack_pointer, uint8_t status, uint8_t cycle)
{
    return (g_test.processor.state.accumulator.low == accumulator)
            && (g_test.processor.state.index.x.low == index_x)
            && (g_test.processor.state.index.y.low == index_y)
            && (g_test.processor.state.program_counter.word == program_counter)
            && (g_test.processor.state.stack_pointer.low == stack_pointer)
            && (g_test.processor.state.status.raw == status)
            && (g_test.processor.cycle == cycle);
}

static nesl_error_e NESL_TestProcessorCycle(void)
{
    uint64_t cycle;
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, false)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.cycle = 10;

    for(cycle = 1; cycle <= 3; ++cycle) {
        NESL_ProcessorCycle(&g_test.processor, cycle);

        if(NESL_ASSERT(g_test.processor.cycle == ((cycle < 3) ? 10 : 9))) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInit(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, false)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_ProcessorInit(&g_test.processor) == NESL_SUCCESS)) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionArithmetic(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x6D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0x6D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x50)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA1, 0x00, 0x00, 0xABD0, 0xFD, 0xF4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0x6D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xD0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x21, 0x00, 0x00, 0xABD0, 0xFD, 0x35, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x6D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xE0, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x6D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x90)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0x00, 0x00, 0xABD0, 0xFD, 0x75, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0x6D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xD0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA1, 0x00, 0x00, 0xABD0, 0xFD, 0xB5, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x7D, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0x03, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x7D, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0xAC, 0x00, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x79, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0x00, 0x03, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x79, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0x00, 0xAC, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x69, NESL_OPERAND_IMMEDIATE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x61, NESL_OPERAND_INDIRECT_X, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x71, NESL_OPERAND_INDIRECT_Y, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0x00, 0x03, 0xABCF, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x71, NESL_OPERAND_INDIRECT_Y, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0x00, 0xAC, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x65, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x75, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x60, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xED, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0xED, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xB0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x00, 0x00, 0xABD0, 0xFD, 0xF4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0xED, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x30)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x20, 0x00, 0x00, 0xABD0, 0xFD, 0x35, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xED, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xDF, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xED, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x70)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0x00, 0x00, 0xABD0, 0xFD, 0x75, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0xED, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x30)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x00, 0x00, 0xABD0, 0xFD, 0xB5, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xFD, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0x03, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xFD, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0xAC, 0x00, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xF9, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0x00, 0x03, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xF9, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0x00, 0xAC, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xE9, NESL_OPERAND_IMMEDIATE, 0, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xE1, NESL_OPERAND_INDIRECT_Y, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xF1, NESL_OPERAND_INDIRECT_Y, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0x00, 0x03, 0xABCF, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xF1, NESL_OPERAND_INDIRECT_Y, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0x00, 0xAC, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xE5, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xF5, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5F, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionBit(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x2C, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x36, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0x2C, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x10, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0x2C, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xC0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x10, 0x00, 0x00, 0xABD0, 0xFD, 0xF6, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0x24, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xC0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x10, 0x00, 0x00, 0xABCF, 0xFD, 0xF6, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionBranch(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x90, NESL_OPERAND_RELATIVE, 0, 0, 0xFA)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABC9, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x90, NESL_OPERAND_RELATIVE, 0, 0, 0x7F)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xAC4E, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x90, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0x90, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x35, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0xB0, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x35, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0xB0, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.zero = true;

    if((result = NESL_TestInstruction(0xABCD, 0xF0, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x36, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.zero = false;

    if((result = NESL_TestInstruction(0xABCD, 0xF0, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.negative = true;

    if((result = NESL_TestInstruction(0xABCD, 0x30, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.negative = false;

    if((result = NESL_TestInstruction(0xABCD, 0x30, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.zero = false;

    if((result = NESL_TestInstruction(0xABCD, 0xD0, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.zero = true;

    if((result = NESL_TestInstruction(0xABCD, 0xD0, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.negative = false;

    if((result = NESL_TestInstruction(0xABCD, 0x10, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.negative = true;

    if((result = NESL_TestInstruction(0xABCD, 0x10, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.overflow = false;

    if((result = NESL_TestInstruction(0xABCD, 0x50, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.overflow = true;

    if((result = NESL_TestInstruction(0xABCD, 0x50, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x74, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.overflow = true;

    if((result = NESL_TestInstruction(0xABCD, 0x70, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x74, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.overflow = false;

    if((result = NESL_TestInstruction(0xABCD, 0x70, NESL_OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionBreakpoint(void)
{
    uint16_t address;
    nesl_error_e result;
    nesl_processor_status_t status = {};

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    address = 0x5678;
    g_test.processor.state.status.breakpoint = false;
    g_test.processor.state.status.interrupt_disable = false;
    status.raw = g_test.processor.state.status.raw;
    status.breakpoint = true;
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0xFFFE, address);
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0xFFFF, address >> 8);

    if((result = NESL_TestInstruction(0xABCD, 0x00, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, address, 0xFA, 0x34, 6) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 1) == status.raw)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 2) == 0xCF)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 3) == 0xAB))) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionClear(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0x18, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.decimal = true;

    if((result = NESL_TestInstruction(0xABCD, 0xD8, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.interrupt_disable = true;

    if((result = NESL_TestInstruction(0xABCD, 0x58, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x30, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.overflow = true;

    if((result = NESL_TestInstruction(0xABCD, 0xB8, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionCompare(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;

    if((result = NESL_TestInstruction(0xABCD, 0xCD, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xCD;

    if((result = NESL_TestInstruction(0xABCD, 0xCD, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xCD, 0x00, 0x00, 0xABD0, 0xFD, 0x35, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xFF;

    if((result = NESL_TestInstruction(0xABCD, 0xCD, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x05)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFF, 0x00, 0x00, 0xABD0, 0xFD, 0xB5, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0xCD, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0x00, 0xABD0, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xDD, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x03, 0x00, 0xABD0, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0xDD, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0xAC, 0x00, 0xABD0, 0xFD, 0x37, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xD9, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0x03, 0xABD0, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0xD9, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0xAC, 0xABD0, 0xFD, 0x37, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0xC9, NESL_OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0x00, 0xABCF, 0xFD, 0x37, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xC1, NESL_OPERAND_INDIRECT_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0x37, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xD1, NESL_OPERAND_INDIRECT_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0x03, 0xABCF, 0xFD, 0x37, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0xD1, NESL_OPERAND_INDIRECT_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0xAC, 0xABCF, 0xFD, 0x37, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0xC5, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0x00, 0xABCF, 0xFD, 0x37, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xD5, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0xEC, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0xAB, 0x00, 0xABD0, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0xE0, NESL_OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0xAB, 0x00, 0xABCF, 0xFD, 0x37, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0xE4, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0xAB, 0x00, 0xABCF, 0xFD, 0x37, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0xCC, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0xAB, 0xABD0, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0xC0, NESL_OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0xAB, 0xABCF, 0xFD, 0x37, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0xC4, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0xAB, 0xABCF, 0xFD, 0x37, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionDecrement(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xCE, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xDE, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0xB4, 6) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4458) == 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xC6, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 4) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0055) == 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xD6, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0058) == 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x00;

    if((result = NESL_TestInstruction(0xABCD, 0xCA, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0xFF, 0x00, 0xABCE, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x01;

    if((result = NESL_TestInstruction(0xABCD, 0xCA, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0xCA, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x0F, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x00;

    if((result = NESL_TestInstruction(0xABCD, 0x88, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0xFF, 0xABCE, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x01;

    if((result = NESL_TestInstruction(0xABCD, 0x88, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0x88, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x0F, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionIncrement(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xEE, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x36, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xFE, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0x36, 6) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4458) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xE6, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x36, 4) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0055) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xF6, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0x36, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0058) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x7F;

    if((result = NESL_TestInstruction(0xABCD, 0xE8, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x80, 0x00, 0xABCE, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xFF;

    if((result = NESL_TestInstruction(0xABCD, 0xE8, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0xE8, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x11, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x7F;

    if((result = NESL_TestInstruction(0xABCD, 0xC8, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x80, 0xABCE, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xFF;

    if((result = NESL_TestInstruction(0xABCD, 0xC8, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0xC8, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x11, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionJump(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x4C, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0x4455, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x6C, NESL_OPERAND_INDIRECT, 0x4455, 0xAABB, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0x4455, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x20, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0x4455, 0xFB, 0x34, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x01FC) == 0xCF)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x01FD) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionLoad(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xAD, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xBD, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x03, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0xBD, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0xAC, 0x00, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xB9, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0x03, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0xB9, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0xAC, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xA9, NESL_OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xA1, NESL_OPERAND_INDIRECT_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xB1, NESL_OPERAND_INDIRECT_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0x03, 0xABCF, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0xB1, NESL_OPERAND_INDIRECT_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0xAC, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xA5, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xB5, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xAE, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0xAB, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xBE, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0xAB, 0x03, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xA2, NESL_OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0xAB, 0x00, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xA6, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0xAB, 0x00, 0xABCF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xB6, NESL_OPERAND_ZEROPAGE_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0xAB, 0x03, 0xABCF, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xAC, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0xAB, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xBC, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x03, 0xAB, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xA0, NESL_OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0xAB, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xA4, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0xAB, 0xABCF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0xB4, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x03, 0xAB, 0xABCF, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionLogical(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xFF;

    if((result = NESL_TestInstruction(0xABCD, 0x2D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x36, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = NESL_TestInstruction(0xABCD, 0x2D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x3D, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x03, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0x3D, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0xAC, 0x00, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x39, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x00, 0x03, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0x39, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x00, 0xAC, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = NESL_TestInstruction(0xABCD, 0x29, NESL_OPERAND_IMMEDIATE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x21, NESL_OPERAND_INDIRECT_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x31, NESL_OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x00, 0x03, 0xABCF, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0x31, NESL_OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x00, 0xAC, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = NESL_TestInstruction(0xABCD, 0x25, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x35, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xA0, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xFF;

    if((result = NESL_TestInstruction(0xABCD, 0x4D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFF, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = NESL_TestInstruction(0xABCD, 0x4D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5B, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x5D, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5B, 0x03, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0x5D, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5B, 0xAC, 0x00, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x59, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5B, 0x00, 0x03, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0x59, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5B, 0x00, 0xAC, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = NESL_TestInstruction(0xABCD, 0x49, NESL_OPERAND_IMMEDIATE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5B, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x41, NESL_OPERAND_INDIRECT_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5B, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x51, NESL_OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5B, 0x00, 0x03, 0xABCF, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0x51, NESL_OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5B, 0x00, 0xAC, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = NESL_TestInstruction(0xABCD, 0x45, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5B, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x55, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x5B, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xFF;

    if((result = NESL_TestInstruction(0xABCD, 0x0D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFF, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = NESL_TestInstruction(0xABCD, 0x0D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFB, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x1D, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFB, 0x03, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0x1D, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFB, 0xAC, 0x00, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x19, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFB, 0x00, 0x03, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0x19, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFB, 0x00, 0xAC, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = NESL_TestInstruction(0xABCD, 0x09, NESL_OPERAND_IMMEDIATE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFB, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x01, NESL_OPERAND_INDIRECT_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFB, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x11, NESL_OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFB, 0x00, 0x03, 0xABCF, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = NESL_TestInstruction(0xABCD, 0x11, NESL_OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFB, 0x00, 0xAC, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = NESL_TestInstruction(0xABCD, 0x05, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFB, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x15, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFB, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionNoOperation(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0xEA, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionPull(void)
{
    nesl_error_e result;
    nesl_processor_status_t status = {};

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FE, 0x00);

    if((result = NESL_TestInstruction(0xABCD, 0x68, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFE, 0x36, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FE, 0xAA);

    if((result = NESL_TestInstruction(0xABCD, 0x68, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xAA, 0x00, 0x00, 0xABCE, 0xFE, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FE, 0x10);

    if((result = NESL_TestInstruction(0xABCD, 0x68, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x10, 0x00, 0x00, 0xABCE, 0xFE, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    status.raw = 0x24;
    g_test.processor.state.status.raw = status.raw;
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FE, status.raw);

    if((result = NESL_TestInstruction(0xABCD, 0x28, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    status.breakpoint = false;

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFE, status.raw, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    status.raw = 0x34;
    g_test.processor.state.status.raw = status.raw;
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FE, status.raw);

    if((result = NESL_TestInstruction(0xABCD, 0x28, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    status.breakpoint = true;

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFE, status.raw, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionPush(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0x48, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x10, 0x00, 0x00, 0xABCE, 0xFC, 0x34, 2) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x01FD) == 0x10))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.raw = 0x24;

    if((result = NESL_TestInstruction(0xABCD, 0x08, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFC, 0x24, 2) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x01FD) == 0x24))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionReturn(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0xFB;
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FC, 0x14);
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FD, 0x44);
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FE, 0x55);

    if((result = NESL_TestInstruction(0xABCD, 0x40, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0x5544, 0xFE, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0xFB;
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FC, 0x04);
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FD, 0x44);
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FE, 0x55);

    if((result = NESL_TestInstruction(0xABCD, 0x40, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0x5544, 0xFE, 0x24, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FE, 0x43);
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x01FF, 0x55);

    if((result = NESL_TestInstruction(0xABCD, 0x60, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0x5544, 0xFF, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionRotate(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0x2E, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0x01))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x2E, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0xB5, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x3E, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0xB5, 6) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4458) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x2A, NESL_OPERAND_ACCUMULATOR, 0, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFE, 0x00, 0x00, 0xABCE, 0xFD, 0xB5, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x26, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0xB5, 4) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0055) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x36, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0xB5, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0058) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0x6E, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0x80))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x6E, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x35, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x7E, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0x35, 6) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4458) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x6A, NESL_OPERAND_ACCUMULATOR, 0, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x7F, 0x00, 0x00, 0xABCE, 0xFD, 0x35, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x66, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x35, 4) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0055) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x76, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0x35, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0058) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionSet(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = false;

    if((result = NESL_TestInstruction(0xABCD, 0x38, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x35, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.decimal = false;

    if((result = NESL_TestInstruction(0xABCD, 0xF8, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x3C, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.interrupt_disable = false;

    if((result = NESL_TestInstruction(0xABCD, 0x78, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionShift(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0x0E, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x36, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x0E, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0xB5, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x1E, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0xB5, 6) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4458) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x0A, NESL_OPERAND_ACCUMULATOR, 0, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xFE, 0x00, 0x00, 0xABCE, 0xFD, 0xB5, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x06, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0xB5, 4) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0055) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x16, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0xB5, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0058) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = NESL_TestInstruction(0xABCD, 0x4E, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x36, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x4E, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x35, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x5E, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0x35, 6) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4458) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x4A, NESL_OPERAND_ACCUMULATOR, 0, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x7F, 0x00, 0x00, 0xABCE, 0xFD, 0x35, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = NESL_TestInstruction(0xABCD, 0x46, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x35, 4) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0055) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x56, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0x35, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0058) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionStore(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0x8D, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0xAB, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x9D, NESL_OPERAND_ABSOLUTE_X, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0xAB, 0x03, 0x00, 0xABD0, 0xFD, 0x34, 4) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4458) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x99, NESL_OPERAND_ABSOLUTE_Y, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0xAB, 0x00, 0x03, 0xABD0, 0xFD, 0x34, 4) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4458) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x81, NESL_OPERAND_INDIRECT_X, 0x4455, 0x0032, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0035) == 0x55)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0036) == 0x44)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x91, NESL_OPERAND_INDIRECT_Y, 0x4455, 0x0032, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0xAB, 0x00, 0x03, 0xABCF, 0xFD, 0x34, 5) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0032) == 0x55)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0033) == 0x44)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4458) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0x85, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0xAB, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 2) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0055) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x95, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 3) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0058) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0x8E, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0xAB, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0x86, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0xAB, 0x00, 0xABCF, 0xFD, 0x34, 2) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0055) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;
    g_test.processor.state.index.y.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x96, NESL_OPERAND_ZEROPAGE_Y, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0xAB, 0x03, 0xABCF, 0xFD, 0x34, 3) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0058) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0x8C, NESL_OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0xAB, 0xABD0, 0xFD, 0x34, 3) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x4455) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;

    if((result = NESL_TestInstruction(0xABCD, 0x84, NESL_OPERAND_ZEROPAGE, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0xAB, 0xABCF, 0xFD, 0x34, 2) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0055) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = NESL_TestInstruction(0xABCD, 0x94, NESL_OPERAND_ZEROPAGE_X, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x03, 0xAB, 0xABCF, 0xFD, 0x34, 3) == true)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, 0x0058) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionTransfer(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;
    g_test.processor.state.index.x.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0xAA, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0xAA, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0xF0, 0xF0, 0x00, 0xABCE, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;
    g_test.processor.state.index.x.low = 0x00;

    if((result = NESL_TestInstruction(0xABCD, 0xAA, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x10, 0x10, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;
    g_test.processor.state.index.y.low = 0x00;

    if((result = NESL_TestInstruction(0xABCD, 0xA8, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x10, 0x00, 0x10, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0x10;
    g_test.processor.state.index.x.low = 0x00;

    if((result = NESL_TestInstruction(0xABCD, 0xBA, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x10, 0x00, 0xABCE, 0x10, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;
    g_test.processor.state.index.x.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0x8A, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x10, 0x10, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0x10;
    g_test.processor.state.index.x.low = 0x00;

    if((result = NESL_TestInstruction(0xABCD, 0x9A, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0x00, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0x00;
    g_test.processor.state.index.x.low = 0xF0;

    if((result = NESL_TestInstruction(0xABCD, 0x9A, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0xF0, 0x00, 0xABCE, 0xF0, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0x00;
    g_test.processor.state.index.x.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0x9A, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x00, 0x10, 0x00, 0xABCE, 0x10, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;
    g_test.processor.state.index.y.low = 0x10;

    if((result = NESL_TestInstruction(0xABCD, 0x98, NESL_OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(NESL_ASSERT(NESL_TestValidate(0x10, 0x00, 0x10, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInstructionUnsupported(void)
{
    static const nesl_instruction_t UNSUPPORTED[] = {
        { 0x02, NESL_OPERAND_IMPLIED, 2 }, { 0x03, NESL_OPERAND_INDIRECT_X, 8 }, { 0x04, NESL_OPERAND_ZEROPAGE, 3 }, { 0x07, NESL_OPERAND_ZEROPAGE, 5 },
        { 0x0B, NESL_OPERAND_IMMEDIATE, 2 }, { 0x0C, NESL_OPERAND_ABSOLUTE, 4 }, { 0x0F, NESL_OPERAND_ABSOLUTE, 6 }, { 0x12, NESL_OPERAND_IMPLIED, 2 },
        { 0x13, NESL_OPERAND_INDIRECT_Y, 8 }, { 0x14, NESL_OPERAND_ZEROPAGE_X, 4 }, { 0x17, NESL_OPERAND_ZEROPAGE_X, 6 }, { 0x1A, NESL_OPERAND_IMMEDIATE, 2 },
        { 0x1B, NESL_OPERAND_ABSOLUTE_Y, 7 }, { 0x1C, NESL_OPERAND_ABSOLUTE_X, 4 }, { 0x1F, NESL_OPERAND_ABSOLUTE_X, 7 }, { 0x22, NESL_OPERAND_IMPLIED, 2 },
        { 0x23, NESL_OPERAND_INDIRECT_X, 8 }, { 0x27, NESL_OPERAND_ZEROPAGE, 5 }, { 0x2B, NESL_OPERAND_IMMEDIATE, 2 }, { 0x2F, NESL_OPERAND_ABSOLUTE, 6 },
        { 0x32, NESL_OPERAND_IMPLIED, 2 }, { 0x33, NESL_OPERAND_INDIRECT_Y, 8 }, { 0x34, NESL_OPERAND_ZEROPAGE_X, 4 }, { 0x37, NESL_OPERAND_ZEROPAGE_X, 6 },
        { 0x3A, NESL_OPERAND_IMMEDIATE, 2 }, { 0x3B, NESL_OPERAND_ABSOLUTE_Y, 7 }, { 0x3C, NESL_OPERAND_ABSOLUTE_X, 4 }, { 0x3F, NESL_OPERAND_ABSOLUTE_X, 7 },
        { 0x42, NESL_OPERAND_IMPLIED, 2 }, { 0x43, NESL_OPERAND_INDIRECT_X, 8 }, { 0x44, NESL_OPERAND_ZEROPAGE, 3 }, { 0x47, NESL_OPERAND_ZEROPAGE, 5 },
        { 0x4B, NESL_OPERAND_IMMEDIATE, 2 }, { 0x4F, NESL_OPERAND_ABSOLUTE, 6 }, { 0x52, NESL_OPERAND_IMPLIED, 2 }, { 0x53, NESL_OPERAND_INDIRECT_Y, 8 },
        { 0x54, NESL_OPERAND_ZEROPAGE_X, 4 }, { 0x57, NESL_OPERAND_ZEROPAGE_X, 6 }, { 0x5A, NESL_OPERAND_IMMEDIATE, 2 }, { 0x5B, NESL_OPERAND_ABSOLUTE_Y, 7 },
        { 0x5C, NESL_OPERAND_ABSOLUTE_X, 4 }, { 0x5F, NESL_OPERAND_ABSOLUTE_X, 7 }, { 0x62, NESL_OPERAND_IMPLIED, 2 }, { 0x63, NESL_OPERAND_INDIRECT_X, 8 },
        { 0x64, NESL_OPERAND_ZEROPAGE, 3 }, { 0x67, NESL_OPERAND_ZEROPAGE, 5 }, { 0x6B, NESL_OPERAND_IMMEDIATE, 2 }, { 0x6F, NESL_OPERAND_ABSOLUTE, 6 },
        { 0x72, NESL_OPERAND_IMPLIED, 2 }, { 0x73, NESL_OPERAND_INDIRECT_Y, 8 }, { 0x74, NESL_OPERAND_ZEROPAGE_X, 4 }, { 0x77, NESL_OPERAND_ZEROPAGE_X, 6 },
        { 0x7A, NESL_OPERAND_IMMEDIATE, 2 }, { 0x7B, NESL_OPERAND_ABSOLUTE_Y, 7 }, { 0x7C, NESL_OPERAND_ABSOLUTE_X, 4 }, { 0x7F, NESL_OPERAND_ABSOLUTE_X, 7 },
        { 0x82, NESL_OPERAND_IMMEDIATE, 2 }, { 0x83, NESL_OPERAND_INDIRECT_X, 6 }, { 0x87, NESL_OPERAND_ZEROPAGE, 3 }, { 0x89, NESL_OPERAND_IMMEDIATE, 2 },
        { 0x8B, NESL_OPERAND_IMMEDIATE, 2 }, { 0x8F, NESL_OPERAND_ABSOLUTE, 4 }, { 0x92, NESL_OPERAND_IMPLIED, 2 }, { 0x93, NESL_OPERAND_INDIRECT_Y, 6 },
        { 0x97, NESL_OPERAND_ZEROPAGE_Y, 4 }, { 0x9B, NESL_OPERAND_ABSOLUTE_Y, 5 }, { 0x9C, NESL_OPERAND_ABSOLUTE_X, 5 }, { 0x9E, NESL_OPERAND_ABSOLUTE_Y, 5 },
        { 0x9F, NESL_OPERAND_ABSOLUTE_Y, 5 }, { 0xA3, NESL_OPERAND_INDIRECT_X, 6 }, { 0xA7, NESL_OPERAND_ZEROPAGE, 3 }, { 0xAB, NESL_OPERAND_IMMEDIATE, 2 },
        { 0xAF, NESL_OPERAND_ABSOLUTE, 4 }, { 0xB2, NESL_OPERAND_IMPLIED, 2 }, { 0xB3, NESL_OPERAND_INDIRECT_Y, 5 }, { 0xB7, NESL_OPERAND_ZEROPAGE_Y, 4 },
        { 0xBB, NESL_OPERAND_ABSOLUTE_Y, 4 }, { 0xBF, NESL_OPERAND_ABSOLUTE_Y, 4 }, { 0xC2, NESL_OPERAND_IMMEDIATE, 2 }, { 0xC3, NESL_OPERAND_INDIRECT_X, 8 },
        { 0xC7, NESL_OPERAND_ZEROPAGE, 5 }, { 0xCB, NESL_OPERAND_IMMEDIATE, 2 }, { 0xCF, NESL_OPERAND_ABSOLUTE, 6 }, { 0xD2, NESL_OPERAND_IMPLIED, 2 },
        { 0xD3, NESL_OPERAND_INDIRECT_Y, 8 }, { 0xD4, NESL_OPERAND_ZEROPAGE_X, 4 }, { 0xD7, NESL_OPERAND_ZEROPAGE_X, 6 }, { 0xDA, NESL_OPERAND_IMMEDIATE, 2 },
        { 0xDB, NESL_OPERAND_ABSOLUTE_Y, 7 }, { 0xDC, NESL_OPERAND_ABSOLUTE_X, 4 }, { 0xDF, NESL_OPERAND_ABSOLUTE_X, 7 }, { 0xE2, NESL_OPERAND_IMMEDIATE, 2 },
        { 0xE3, NESL_OPERAND_INDIRECT_X, 8 }, { 0xE7, NESL_OPERAND_ZEROPAGE, 5 }, { 0xEB, NESL_OPERAND_IMMEDIATE, 2 }, { 0xEF, NESL_OPERAND_ABSOLUTE, 6 },
        { 0xF2, NESL_OPERAND_IMPLIED, 2 }, { 0xF3, NESL_OPERAND_INDIRECT_Y, 8 }, { 0xF4, NESL_OPERAND_ZEROPAGE_X, 4 }, { 0xF7, NESL_OPERAND_ZEROPAGE_X, 6 },
        { 0xFA, NESL_OPERAND_IMMEDIATE, 2 }, { 0xFB, NESL_OPERAND_ABSOLUTE_Y, 7 }, { 0xFC, NESL_OPERAND_ABSOLUTE_X, 4 }, { 0xFF, NESL_OPERAND_ABSOLUTE_X, 7 },
        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < (sizeof(UNSUPPORTED) / sizeof(nesl_instruction_t)); ++index) {
        uint16_t address = 0xABCD;
        const nesl_instruction_t *unsupported = &UNSUPPORTED[index];

        if((result = NESL_TestInit(address, true)) == NESL_FAILURE) {
            goto exit;
        }

        if((result = NESL_TestInstruction(address++, unsupported->type, unsupported->mode, 0, 0, 0)) == NESL_FAILURE) {
            goto exit;
        }

        switch(unsupported->mode) {
            case NESL_OPERAND_ABSOLUTE:
            case NESL_OPERAND_ABSOLUTE_X:
            case NESL_OPERAND_ABSOLUTE_Y:
            case NESL_OPERAND_INDIRECT:
                address += 2;
                break;
            case NESL_OPERAND_IMMEDIATE:
            case NESL_OPERAND_INDIRECT_X:
            case NESL_OPERAND_INDIRECT_Y:
            case NESL_OPERAND_RELATIVE:
            case NESL_OPERAND_ZEROPAGE:
            case NESL_OPERAND_ZEROPAGE_X:
            case NESL_OPERAND_ZEROPAGE_Y:
                ++address;
                break;
            default:
                break;
        }

        if(NESL_ASSERT(NESL_TestValidate(0x00, 0x00, 0x00, address, 0xFD, 0x34, unsupported->cycles - 1) == true)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorInterrupt(void)
{
    uint16_t address;
    nesl_error_e result;
    nesl_processor_status_t status = {};

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    NESL_ProcessorInterrupt(&g_test.processor, false);

    if(NESL_ASSERT(g_test.processor.interrupt.non_maskable)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    NESL_ProcessorInterrupt(&g_test.processor, true);

    if(NESL_ASSERT(g_test.processor.interrupt.maskable)) {
        result = NESL_FAILURE;
        goto exit;
    }

   if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    address = 0x1234;
    g_test.processor.cycle = 0;
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0xFFFA, address);
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0xFFFB, address >> 8);
    NESL_ProcessorInterrupt(&g_test.processor, false);
    NESL_ProcessorCycle(&g_test.processor, 0);

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, address, 0xFA, 0x34, 6) == true)
            && (g_test.processor.interrupt.non_maskable == false)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 1) == g_test.processor.state.status.raw)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 2) == 0xCD)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 3) == 0xAB))) {
        result = NESL_SUCCESS;
        goto exit;
    }

    g_test.processor.cycle = 0;
    g_test.processor.state.status.interrupt_disable = false;
    status.raw = g_test.processor.state.status.raw;
    NESL_ProcessorInterrupt(&g_test.processor, false);
    NESL_ProcessorCycle(&g_test.processor, 0);

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, address, 0xF7, 0x34, 6) == true)
            && (g_test.processor.interrupt.non_maskable == false)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 1) == status.raw)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 2) == 0x34)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 3) == 0x12))) {
        result = NESL_SUCCESS;
        goto exit;
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    address = 0x5678;
    g_test.processor.cycle = 0;
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0xABCD, 0xEA);
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0xFFFE, address);
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0xFFFF, address >> 8);
    NESL_ProcessorInterrupt(&g_test.processor, true);
    NESL_ProcessorCycle(&g_test.processor, 0);

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)
            && (g_test.processor.interrupt.maskable == true))) {
        result = NESL_SUCCESS;
        goto exit;
    }

    g_test.processor.cycle = 0;
    g_test.processor.state.status.interrupt_disable = false;
    status.raw = g_test.processor.state.status.raw;
    NESL_ProcessorInterrupt(&g_test.processor, true);
    NESL_ProcessorCycle(&g_test.processor, 0);

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, address, 0xFA, 0x34, 6) == true)
            && (g_test.processor.interrupt.maskable == false)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 1) == status.raw)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 2) == 0xCE)
            && (NESL_BusRead(NESL_BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 3) == 0xAB))) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorRead(void)
{
    uint8_t data = 0;
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    for(uint16_t address = 0x0000; address < 0x1FFF; ++address, ++data) {
        g_test.processor.ram[address & 0x07FF] = data;

        if(NESL_ASSERT(NESL_ProcessorRead(&g_test.processor, address) == data)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorReset(void)
{
    nesl_error_e result;
    uint16_t address = 0xDCBA;

    if((result = NESL_TestInit(0xABCD, false)) == NESL_FAILURE) {
        goto exit;
    }

    NESL_BusWrite(NESL_BUS_PROCESSOR, 0xFFFC, address);
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0xFFFD, address >> 8);
    NESL_ProcessorReset(&g_test.processor);

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, address, 0xFD, 0x34, 7) == true)
            && (g_test.processor.interrupt.raw == 0)
            && (g_test.processor.transfer.destination.word == 0)
            && (g_test.processor.transfer.source.word == 0))) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorTransfer(void)
{
    uint64_t cycle;
    uint16_t address;
    nesl_error_e result;
    uint8_t data = 0, page = 0xAB;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    for(address = (page << 8); address <= ((page << 8) | 0xFF); ++address) {
        NESL_BusWrite(NESL_BUS_PROCESSOR, address, data++);
    }

    cycle = 0;
    g_test.processor.cycle = 0;
    NESL_ProcessorWrite(&g_test.processor, 0x4014, page);

    if(NESL_ASSERT((g_test.processor.interrupt.transfer == true)
            && (g_test.processor.interrupt.transfer_sync == true)
            && (g_test.processor.transfer.source.word == (page << 8))
            && (g_test.processor.transfer.destination.word == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_ProcessorCycle(&g_test.processor, cycle);
    cycle += 3;
    NESL_ProcessorCycle(&g_test.processor, cycle);
    cycle += 3;

    for(address = 0; address <= 0xFF; ++address) {
        NESL_ProcessorCycle(&g_test.processor, cycle);
        cycle += 3;

        NESL_ProcessorCycle(&g_test.processor, cycle);
        cycle += 3;

        if(NESL_ASSERT((g_test.processor.interrupt.transfer == (address < 0xFF))
                && (g_test.processor.interrupt.transfer_sync == false)
                && (g_test.processor.transfer.source.word == ((address < 0xFF) ? ((page << 8) + address + 1) : 0))
                && (g_test.processor.transfer.destination.word == ((address < 0xFF) ? address + 1 : 0))
                && (NESL_BusRead(NESL_BUS_VIDEO_OAM, address) == NESL_BusRead(NESL_BUS_PROCESSOR, (page << 8) + address)))) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    for(address = (page << 8); address <= ((page << 8) | 0xFF); ++address) {
        NESL_BusWrite(NESL_BUS_PROCESSOR, address, data++);
    }

    cycle = 3;

    g_test.processor.cycle = 0;
    NESL_ProcessorWrite(&g_test.processor, 0x4014, page);

    if(NESL_ASSERT((g_test.processor.interrupt.transfer == true)
            && (g_test.processor.interrupt.transfer_sync == true)
            && (g_test.processor.transfer.source.word == (page << 8))
            && (g_test.processor.transfer.destination.word == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

    NESL_ProcessorCycle(&g_test.processor, cycle);
    cycle += 3;

    for(address = 0; address <= 0xFF; ++address) {
        NESL_ProcessorCycle(&g_test.processor, cycle);
        cycle += 3;

        NESL_ProcessorCycle(&g_test.processor, cycle);
        cycle += 3;

        if(NESL_ASSERT((g_test.processor.interrupt.transfer == (address < 0xFF))
                && (g_test.processor.interrupt.transfer_sync == false)
                && (g_test.processor.transfer.source.word == ((address < 0xFF) ? ((page << 8) + address + 1) : 0))
                && (g_test.processor.transfer.destination.word == ((address < 0xFF) ? address + 1 : 0))
                && (NESL_BusRead(NESL_BUS_VIDEO_OAM, address) == NESL_BusRead(NESL_BUS_PROCESSOR, (page << 8) + address)))) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorUninit(void)
{
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    NESL_ProcessorUninit(&g_test.processor);

    if(NESL_ASSERT((NESL_TestValidate(0x00, 0x00, 0x00, 0x0000, 0x00, 0x00, 0) == true)
            && (g_test.processor.interrupt.raw == 0)
            && (g_test.processor.transfer.destination.word == 0)
            && (g_test.processor.transfer.source.word == 0))) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    NESL_TEST_RESULT(result);

    return result;
}

static nesl_error_e NESL_TestProcessorWrite(void)
{
    uint8_t data = 0;
    nesl_error_e result;

    if((result = NESL_TestInit(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    for(uint16_t address = 0x0000; address < 0x1FFF; ++address, ++data) {
        NESL_ProcessorWrite(&g_test.processor, address, data);

        if(NESL_ASSERT(g_test.processor.ram[address & 0x07FF] == data)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    data = 0xAB;
    NESL_ProcessorWrite(&g_test.processor, 0x4014, data);

    if(NESL_ASSERT((g_test.processor.interrupt.transfer == true)
            && (g_test.processor.interrupt.transfer_sync == true)
            && (g_test.processor.transfer.source.word == (data << 8))
            && (g_test.processor.transfer.destination.word == 0))) {
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
        NESL_TestProcessorCycle, NESL_TestProcessorInit, NESL_TestProcessorInstructionArithmetic, NESL_TestProcessorInstructionBit,
        NESL_TestProcessorInstructionBranch, NESL_TestProcessorInstructionBreakpoint, NESL_TestProcessorInstructionClear, NESL_TestProcessorInstructionCompare,
        NESL_TestProcessorInstructionDecrement, NESL_TestProcessorInstructionIncrement, NESL_TestProcessorInstructionJump, NESL_TestProcessorInstructionLoad,
        NESL_TestProcessorInstructionLogical, NESL_TestProcessorInstructionNoOperation, NESL_TestProcessorInstructionPull, NESL_TestProcessorInstructionPush,
        NESL_TestProcessorInstructionReturn, NESL_TestProcessorInstructionRotate, NESL_TestProcessorInstructionSet, NESL_TestProcessorInstructionShift,
        NESL_TestProcessorInstructionStore, NESL_TestProcessorInstructionTransfer, NESL_TestProcessorInstructionUnsupported, NESL_TestProcessorInterrupt,
        NESL_TestProcessorRead, NESL_TestProcessorReset, NESL_TestProcessorTransfer, NESL_TestProcessorUninit,
        NESL_TestProcessorWrite,
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
