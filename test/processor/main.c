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

#include <processor.h>
#include <test.h>

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

uint8_t nesl_bus_read(nesl_bus_e type, uint16_t address)
{
    int result = 0;

    g_test.bus.type = type;
    g_test.bus.address = address;

    switch(type) {
        case BUS_PROCESSOR:
            result = g_test.bus.ram[g_test.bus.address];
            break;
        case BUS_VIDEO_OAM:
            result = g_test.bus.oam[g_test.bus.address];
            break;
        default:
            break;
    }

    return result;
}

void nesl_bus_write(nesl_bus_e type, uint16_t address, uint8_t data)
{
    g_test.bus.type = type;
    g_test.bus.address = address;

    switch(type) {
        case BUS_PROCESSOR:
            g_test.bus.ram[g_test.bus.address] = data;
            break;
        case BUS_VIDEO_OAM:
            g_test.bus.oam[g_test.bus.address] = data;
            break;
        default:
            break;
    }
}

/**
 * @brief Initialize test context.
 * @param address Program counter address
 * @param initialize Initialize processor
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_initialize(uint16_t address, bool initialize)
{
    nesl_error_e result = NESL_SUCCESS;

    memset(&g_test.processor, 0, sizeof(g_test.processor));

    if(initialize) {
        nesl_bus_write(BUS_PROCESSOR, 0xFFFC, address);
        nesl_bus_write(BUS_PROCESSOR, 0xFFFD, address >> 8);

        if((result = nesl_processor_initialize(&g_test.processor)) == NESL_FAILURE) {
            goto exit;
        }
    }

exit:
    return result;
}

/**
 * @brief Execute processor instruction.
 * @param address Program counter address
 * @param opcode Operation
 * @param mode Address mode
 * @param effective Effective address
 * @param indirect Indirect address
 * @param data Immediate data
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_instruction(uint16_t address, uint8_t opcode, nesl_operand_e mode, uint16_t effective, uint16_t indirect, uint8_t data)
{
    nesl_error_e result = NESL_SUCCESS;

    g_test.processor.cycle = 0;
    nesl_bus_write(BUS_PROCESSOR, address, opcode);

    switch(mode) {
        case OPERAND_ABSOLUTE:
            nesl_bus_write(BUS_PROCESSOR, address + 1, effective);
            nesl_bus_write(BUS_PROCESSOR, address + 2, effective >> 8);
            nesl_bus_write(BUS_PROCESSOR, effective, data);
            break;
        case OPERAND_ABSOLUTE_X:
            nesl_bus_write(BUS_PROCESSOR, address + 1, effective);
            nesl_bus_write(BUS_PROCESSOR, address + 2, effective >> 8);
            effective += g_test.processor.state.index.x.low;
            nesl_bus_write(BUS_PROCESSOR, effective, data);
            break;
        case OPERAND_ABSOLUTE_Y:
            nesl_bus_write(BUS_PROCESSOR, address + 1, effective);
            nesl_bus_write(BUS_PROCESSOR, address + 2, effective >> 8);
            effective += g_test.processor.state.index.y.low;
            nesl_bus_write(BUS_PROCESSOR, effective, data);
            break;
        case OPERAND_ACCUMULATOR:
            g_test.processor.state.accumulator.low = data;
            break;
        case OPERAND_IMPLIED:
            break;
        case OPERAND_IMMEDIATE:
        case OPERAND_RELATIVE:
            nesl_bus_write(BUS_PROCESSOR, address + 1, data);
            break;
        case OPERAND_INDIRECT:
            nesl_bus_write(BUS_PROCESSOR, address + 1, indirect);
            nesl_bus_write(BUS_PROCESSOR, address + 2, indirect >> 8);

            if((indirect & 0xFF) == 0xFF) {
                nesl_bus_write(BUS_PROCESSOR, indirect, effective);
                nesl_bus_write(BUS_PROCESSOR, indirect & 0xFF00, effective >> 8);
            } else {
                nesl_bus_write(BUS_PROCESSOR, indirect, effective);
                nesl_bus_write(BUS_PROCESSOR, indirect + 1, effective >> 8);
            }

            nesl_bus_write(BUS_PROCESSOR, effective, data);
            break;
        case OPERAND_INDIRECT_X:
            nesl_bus_write(BUS_PROCESSOR, address + 1, indirect);
            indirect += g_test.processor.state.index.x.low;
            nesl_bus_write(BUS_PROCESSOR, indirect & 0xFF, effective);
            nesl_bus_write(BUS_PROCESSOR, (indirect + 1) & 0xFF, effective >> 8);
            nesl_bus_write(BUS_PROCESSOR, effective, data);
            break;
        case OPERAND_INDIRECT_Y:
            nesl_bus_write(BUS_PROCESSOR, address + 1, indirect);
            nesl_bus_write(BUS_PROCESSOR, indirect & 0xFF, effective);
            nesl_bus_write(BUS_PROCESSOR, (indirect + 1) & 0xFF, effective >> 8);
            effective += g_test.processor.state.index.y.low;
            nesl_bus_write(BUS_PROCESSOR, effective, data);
            break;
        case OPERAND_ZEROPAGE:
            nesl_bus_write(BUS_PROCESSOR, address + 1, effective);
            nesl_bus_write(BUS_PROCESSOR, effective & 0xFF, data);
            break;
        case OPERAND_ZEROPAGE_X:
            nesl_bus_write(BUS_PROCESSOR, address + 1, effective);
            effective += g_test.processor.state.index.x.low;
            nesl_bus_write(BUS_PROCESSOR, effective & 0xFF, data);
            break;
        case OPERAND_ZEROPAGE_Y:
            nesl_bus_write(BUS_PROCESSOR, address + 1, effective);
            effective += g_test.processor.state.index.y.low;
            nesl_bus_write(BUS_PROCESSOR, effective & 0xFF, data);
            break;
        default:
            result = NESL_FAILURE;
            goto exit;
    }

    nesl_processor_cycle(&g_test.processor, 0);

exit:
    return result;
}

/**
 * @brief Validate processor state.
 * @param accumulator Accumulator register
 * @param index_x Index-X register
 * @param index_y Index-Y register
 * @param program_counter Program-counter register
 * @param stack_pointer Stack-pointer register
 * @param status Status register
 * @param cycle Cycle count
 * @return false if state mismatches, true otherwise
 */
static bool nesl_test_validate(uint8_t accumulator, uint8_t index_x, uint8_t index_y, uint16_t program_counter, uint8_t stack_pointer, uint8_t status, uint8_t cycle)
{
    return (g_test.processor.state.accumulator.low == accumulator)
            && (g_test.processor.state.index.x.low == index_x)
            && (g_test.processor.state.index.y.low == index_y)
            && (g_test.processor.state.program_counter.word == program_counter)
            && (g_test.processor.state.stack_pointer.low == stack_pointer)
            && (g_test.processor.state.status.raw == status)
            && (g_test.processor.cycle == cycle);
}

/**
 * @brief Test processor subsystem cycle.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_cycle(void)
{
    uint64_t cycle;
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, false)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.cycle = 10;

    for(cycle = 1; cycle <= 3; ++cycle) {
        nesl_processor_cycle(&g_test.processor, cycle);

        if(ASSERT(g_test.processor.cycle == ((cycle < 3) ? 10 : 9))) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem initialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_initialize(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, false)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_processor_initialize(&g_test.processor) == NESL_SUCCESS)) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem arithmetic instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_arithmetic(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x6D, OPERAND_ABSOLUTE, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0x6D, OPERAND_ABSOLUTE, 0x4455, 0, 0x50)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA1, 0x00, 0x00, 0xABD0, 0xFD, 0xF4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0x6D, OPERAND_ABSOLUTE, 0x4455, 0, 0xD0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x21, 0x00, 0x00, 0xABD0, 0xFD, 0x35, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x6D, OPERAND_ABSOLUTE, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xE0, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x6D, OPERAND_ABSOLUTE, 0x4455, 0, 0x90)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0x00, 0x00, 0xABD0, 0xFD, 0x75, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0x6D, OPERAND_ABSOLUTE, 0x4455, 0, 0xD0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA1, 0x00, 0x00, 0xABD0, 0xFD, 0xB5, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x7D, OPERAND_ABSOLUTE_X, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0x03, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x7D, OPERAND_ABSOLUTE_X, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0xAC, 0x00, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x79, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0x00, 0x03, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x79, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0x00, 0xAC, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x69, OPERAND_IMMEDIATE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x61, OPERAND_INDIRECT_X, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x71, OPERAND_INDIRECT_Y, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0x00, 0x03, 0xABCF, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x71, OPERAND_INDIRECT_Y, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0x00, 0xAC, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x65, OPERAND_ZEROPAGE, 0x0055, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x75, OPERAND_ZEROPAGE_X, 0x0055, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x60, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xED, OPERAND_ABSOLUTE, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0xED, OPERAND_ABSOLUTE, 0x4455, 0, 0xB0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x00, 0x00, 0xABD0, 0xFD, 0xF4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0xED, OPERAND_ABSOLUTE, 0x4455, 0, 0x30)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x20, 0x00, 0x00, 0xABD0, 0xFD, 0x35, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xED, OPERAND_ABSOLUTE, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xDF, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xED, OPERAND_ABSOLUTE, 0x4455, 0, 0x70)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0x00, 0x00, 0xABD0, 0xFD, 0x75, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xD0;
    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0xED, OPERAND_ABSOLUTE, 0x4455, 0, 0x30)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x00, 0x00, 0xABD0, 0xFD, 0xB5, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xFD, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0x03, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xFD, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0xAC, 0x00, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xF9, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0x00, 0x03, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xF9, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0x00, 0xAC, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xE9, OPERAND_IMMEDIATE, 0, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xE1, OPERAND_INDIRECT_Y, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xF1, OPERAND_INDIRECT_Y, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0x00, 0x03, 0xABCF, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.y.low = 0xAC;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xF1, OPERAND_INDIRECT_Y, 0x4455, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0x00, 0xAC, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xE5, OPERAND_ZEROPAGE, 0x0055, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x50;
    g_test.processor.state.index.x.low = 0x03;
    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xF5, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xF0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5F, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem bit instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_bit(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x2C, OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x36, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0x2C, OPERAND_ABSOLUTE, 0x4455, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x10, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0x2C, OPERAND_ABSOLUTE, 0x4455, 0, 0xC0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x10, 0x00, 0x00, 0xABD0, 0xFD, 0xF6, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0x24, OPERAND_ZEROPAGE, 0x0055, 0, 0xC0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x10, 0x00, 0x00, 0xABCF, 0xFD, 0xF6, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem branch instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_branch(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x90, OPERAND_RELATIVE, 0, 0, 0xFA)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABC9, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x90, OPERAND_RELATIVE, 0, 0, 0x7F)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xAC4E, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x90, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0x90, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x35, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0xB0, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x35, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0xB0, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.zero = true;

    if((result = nesl_test_instruction(0xABCD, 0xF0, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x36, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.zero = false;

    if((result = nesl_test_instruction(0xABCD, 0xF0, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.negative = true;

    if((result = nesl_test_instruction(0xABCD, 0x30, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.negative = false;

    if((result = nesl_test_instruction(0xABCD, 0x30, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.zero = false;

    if((result = nesl_test_instruction(0xABCD, 0xD0, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.zero = true;

    if((result = nesl_test_instruction(0xABCD, 0xD0, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.negative = false;

    if((result = nesl_test_instruction(0xABCD, 0x10, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.negative = true;

    if((result = nesl_test_instruction(0xABCD, 0x10, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.overflow = false;

    if((result = nesl_test_instruction(0xABCD, 0x50, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.overflow = true;

    if((result = nesl_test_instruction(0xABCD, 0x50, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x74, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.overflow = true;

    if((result = nesl_test_instruction(0xABCD, 0x70, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABDF, 0xFD, 0x74, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.overflow = false;

    if((result = nesl_test_instruction(0xABCD, 0x70, OPERAND_RELATIVE, 0, 0, 0x10)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem breakpoint instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_breakpoint(void)
{
    uint16_t address;
    nesl_error_e result;
    nesl_processor_status_t status = {};

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    address = 0x5678;
    g_test.processor.state.status.breakpoint = false;
    g_test.processor.state.status.interrupt_disable = false;
    status.raw = g_test.processor.state.status.raw;
    status.breakpoint = true;
    nesl_bus_write(BUS_PROCESSOR, 0xFFFE, address);
    nesl_bus_write(BUS_PROCESSOR, 0xFFFF, address >> 8);

    if((result = nesl_test_instruction(0xABCD, 0x00, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, address, 0xFA, 0x34, 6) == true)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 1) == status.raw)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 2) == 0xCF)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 3) == 0xAB))) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem clear bit instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_clear(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0x18, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.decimal = true;

    if((result = nesl_test_instruction(0xABCD, 0xD8, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.interrupt_disable = true;

    if((result = nesl_test_instruction(0xABCD, 0x58, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x30, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.overflow = true;

    if((result = nesl_test_instruction(0xABCD, 0xB8, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem compare instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_compare(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;

    if((result = nesl_test_instruction(0xABCD, 0xCD, OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xCD;

    if((result = nesl_test_instruction(0xABCD, 0xCD, OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xCD, 0x00, 0x00, 0xABD0, 0xFD, 0x35, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xFF;

    if((result = nesl_test_instruction(0xABCD, 0xCD, OPERAND_ABSOLUTE, 0x4455, 0, 0x05)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFF, 0x00, 0x00, 0xABD0, 0xFD, 0xB5, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0xCD, OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0x00, 0xABD0, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xDD, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x03, 0x00, 0xABD0, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0xDD, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0xAC, 0x00, 0xABD0, 0xFD, 0x37, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xD9, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0x03, 0xABD0, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0xD9, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0xAC, 0xABD0, 0xFD, 0x37, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0xC9, OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0x00, 0xABCF, 0xFD, 0x37, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xC1, OPERAND_INDIRECT_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0x37, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xD1, OPERAND_INDIRECT_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0x03, 0xABCF, 0xFD, 0x37, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0xD1, OPERAND_INDIRECT_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0xAC, 0xABCF, 0xFD, 0x37, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0xC5, OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0x00, 0xABCF, 0xFD, 0x37, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xD5, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0xEC, OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0xAB, 0x00, 0xABD0, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0xE0, OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0xAB, 0x00, 0xABCF, 0xFD, 0x37, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0xE4, OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0xAB, 0x00, 0xABCF, 0xFD, 0x37, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0xCC, OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0xAB, 0xABD0, 0xFD, 0x37, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0xC0, OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0xAB, 0xABCF, 0xFD, 0x37, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0xC4, OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0xAB, 0xABCF, 0xFD, 0x37, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem decrement instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_decrement(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xCE, OPERAND_ABSOLUTE, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xDE, OPERAND_ABSOLUTE_X, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0xB4, 6) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4458) == 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xC6, OPERAND_ZEROPAGE, 0x0055, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 4) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0055) == 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xD6, OPERAND_ZEROPAGE_X, 0x0055, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0058) == 0xFF))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x00;

    if((result = nesl_test_instruction(0xABCD, 0xCA, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0xFF, 0x00, 0xABCE, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x01;

    if((result = nesl_test_instruction(0xABCD, 0xCA, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0xCA, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x0F, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x00;

    if((result = nesl_test_instruction(0xABCD, 0x88, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0xFF, 0xABCE, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x01;

    if((result = nesl_test_instruction(0xABCD, 0x88, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0x88, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x0F, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem increment instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_increment(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xEE, OPERAND_ABSOLUTE, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x36, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xFE, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0x36, 6) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4458) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xE6, OPERAND_ZEROPAGE, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x36, 4) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0055) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xF6, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0x36, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0058) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x7F;

    if((result = nesl_test_instruction(0xABCD, 0xE8, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x80, 0x00, 0xABCE, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xFF;

    if((result = nesl_test_instruction(0xABCD, 0xE8, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0xE8, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x11, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x7F;

    if((result = nesl_test_instruction(0xABCD, 0xC8, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x80, 0xABCE, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xFF;

    if((result = nesl_test_instruction(0xABCD, 0xC8, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0xC8, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x11, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem jump instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_jump(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x4C, OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0x4455, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x6C, OPERAND_INDIRECT, 0x4455, 0xAABB, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0x4455, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x20, OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0x4455, 0xFB, 0x34, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x01FC) == 0xCF)
            && (nesl_bus_read(BUS_PROCESSOR, 0x01FD) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem load instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_load(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xAD, OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xBD, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x03, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0xBD, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0xAC, 0x00, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xB9, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0x03, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0xB9, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0xAC, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xA9, OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xA1, OPERAND_INDIRECT_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xB1, OPERAND_INDIRECT_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0x03, 0xABCF, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0xB1, OPERAND_INDIRECT_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0xAC, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xA5, OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xB5, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xAE, OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0xAB, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xBE, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0xAB, 0x03, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xA2, OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0xAB, 0x00, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xA6, OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0xAB, 0x00, 0xABCF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xB6, OPERAND_ZEROPAGE_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0xAB, 0x03, 0xABCF, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xAC, OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0xAB, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xBC, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x03, 0xAB, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xA0, OPERAND_IMMEDIATE, 0, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0xAB, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xA4, OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0xAB, 0xABCF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0xB4, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x03, 0xAB, 0xABCF, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem logical instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_logical(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xFF;

    if((result = nesl_test_instruction(0xABCD, 0x2D, OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x36, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = nesl_test_instruction(0xABCD, 0x2D, OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x3D, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x03, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0x3D, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0xAC, 0x00, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x39, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x00, 0x03, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0x39, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x00, 0xAC, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = nesl_test_instruction(0xABCD, 0x29, OPERAND_IMMEDIATE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x21, OPERAND_INDIRECT_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x31, OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x00, 0x03, 0xABCF, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0x31, OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x00, 0xAC, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = nesl_test_instruction(0xABCD, 0x25, OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x35, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xA0, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xFF;

    if((result = nesl_test_instruction(0xABCD, 0x4D, OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFF, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = nesl_test_instruction(0xABCD, 0x4D, OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5B, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x5D, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5B, 0x03, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0x5D, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5B, 0xAC, 0x00, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x59, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5B, 0x00, 0x03, 0xABD0, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0x59, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5B, 0x00, 0xAC, 0xABD0, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = nesl_test_instruction(0xABCD, 0x49, OPERAND_IMMEDIATE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5B, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x41, OPERAND_INDIRECT_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5B, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x51, OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5B, 0x00, 0x03, 0xABCF, 0xFD, 0x34, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0x51, OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5B, 0x00, 0xAC, 0xABCF, 0xFD, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = nesl_test_instruction(0xABCD, 0x45, OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5B, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x55, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x5B, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xFF;

    if((result = nesl_test_instruction(0xABCD, 0x0D, OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFF, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = nesl_test_instruction(0xABCD, 0x0D, OPERAND_ABSOLUTE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFB, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x1D, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFB, 0x03, 0x00, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0x1D, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFB, 0xAC, 0x00, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x19, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFB, 0x00, 0x03, 0xABD0, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0x19, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFB, 0x00, 0xAC, 0xABD0, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = nesl_test_instruction(0xABCD, 0x09, OPERAND_IMMEDIATE, 0x4455, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFB, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x01, OPERAND_INDIRECT_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFB, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x11, OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFB, 0x00, 0x03, 0xABCF, 0xFD, 0xB4, 4) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.y.low = 0xAC;

    if((result = nesl_test_instruction(0xABCD, 0x11, OPERAND_INDIRECT_Y, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFB, 0x00, 0xAC, 0xABCF, 0xFD, 0xB4, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;

    if((result = nesl_test_instruction(0xABCD, 0x05, OPERAND_ZEROPAGE, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFB, 0x00, 0x00, 0xABCF, 0xFD, 0xB4, 2) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x15, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xAB)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFB, 0x03, 0x00, 0xABCF, 0xFD, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem no-operation instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_no_operation(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0xEA, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem pull instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_pull(void)
{
    nesl_error_e result;
    nesl_processor_status_t status = {};

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;
    nesl_bus_write(BUS_PROCESSOR, 0x01FE, 0x00);

    if((result = nesl_test_instruction(0xABCD, 0x68, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFE, 0x36, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;
    nesl_bus_write(BUS_PROCESSOR, 0x01FE, 0xAA);

    if((result = nesl_test_instruction(0xABCD, 0x68, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xAA, 0x00, 0x00, 0xABCE, 0xFE, 0xB4, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;
    nesl_bus_write(BUS_PROCESSOR, 0x01FE, 0x10);

    if((result = nesl_test_instruction(0xABCD, 0x68, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x10, 0x00, 0x00, 0xABCE, 0xFE, 0x34, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    status.raw = 0x24;
    g_test.processor.state.status.raw = status.raw;
    nesl_bus_write(BUS_PROCESSOR, 0x01FE, status.raw);

    if((result = nesl_test_instruction(0xABCD, 0x28, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    status.breakpoint = false;

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFE, status.raw, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    status.raw = 0x34;
    g_test.processor.state.status.raw = status.raw;
    nesl_bus_write(BUS_PROCESSOR, 0x01FE, status.raw);

    if((result = nesl_test_instruction(0xABCD, 0x28, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    status.breakpoint = true;

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFE, status.raw, 3) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem push instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_push(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0x48, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x10, 0x00, 0x00, 0xABCE, 0xFC, 0x34, 2) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x01FD) == 0x10))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.raw = 0x24;

    if((result = nesl_test_instruction(0xABCD, 0x08, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFC, 0x24, 2) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x01FD) == 0x24))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem return instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_return(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0xFB;
    nesl_bus_write(BUS_PROCESSOR, 0x01FC, 0x14);
    nesl_bus_write(BUS_PROCESSOR, 0x01FD, 0x44);
    nesl_bus_write(BUS_PROCESSOR, 0x01FE, 0x55);

    if((result = nesl_test_instruction(0xABCD, 0x40, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0x5544, 0xFE, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0xFB;
    nesl_bus_write(BUS_PROCESSOR, 0x01FC, 0x04);
    nesl_bus_write(BUS_PROCESSOR, 0x01FD, 0x44);
    nesl_bus_write(BUS_PROCESSOR, 0x01FE, 0x55);

    if((result = nesl_test_instruction(0xABCD, 0x40, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0x5544, 0xFE, 0x24, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    nesl_bus_write(BUS_PROCESSOR, 0x01FE, 0x43);
    nesl_bus_write(BUS_PROCESSOR, 0x01FF, 0x55);

    if((result = nesl_test_instruction(0xABCD, 0x60, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0x5544, 0xFF, 0x34, 5) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem rotate instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_rotate(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0x2E, OPERAND_ABSOLUTE, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0x01))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x2E, OPERAND_ABSOLUTE, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0xB5, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x3E, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0xB5, 6) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4458) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x2A, OPERAND_ACCUMULATOR, 0, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFE, 0x00, 0x00, 0xABCE, 0xFD, 0xB5, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x26, OPERAND_ZEROPAGE, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0xB5, 4) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0055) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x36, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0xB5, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0058) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0x6E, OPERAND_ABSOLUTE, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0xB4, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0x80))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x6E, OPERAND_ABSOLUTE, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x35, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x7E, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0x35, 6) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4458) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x6A, OPERAND_ACCUMULATOR, 0, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x7F, 0x00, 0x00, 0xABCE, 0xFD, 0x35, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x66, OPERAND_ZEROPAGE, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x35, 4) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0055) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x76, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0x35, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0058) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem set bit instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_set(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = false;

    if((result = nesl_test_instruction(0xABCD, 0x38, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x35, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.decimal = false;

    if((result = nesl_test_instruction(0xABCD, 0xF8, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x3C, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.interrupt_disable = false;

    if((result = nesl_test_instruction(0xABCD, 0x78, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem shift instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_shift(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0x0E, OPERAND_ABSOLUTE, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x36, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x0E, OPERAND_ABSOLUTE, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0xB5, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x1E, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0xB5, 6) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4458) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x0A, OPERAND_ACCUMULATOR, 0, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xFE, 0x00, 0x00, 0xABCE, 0xFD, 0xB5, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x06, OPERAND_ZEROPAGE, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0xB5, 4) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0055) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x16, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0xB5, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0058) == 0xFE))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.status.carry = true;

    if((result = nesl_test_instruction(0xABCD, 0x4E, OPERAND_ABSOLUTE, 0x4455, 0, 0x00)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x36, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0x00))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x4E, OPERAND_ABSOLUTE, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABD0, 0xFD, 0x35, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x5E, OPERAND_ABSOLUTE_X, 0x4455, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABD0, 0xFD, 0x35, 6) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4458) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x4A, OPERAND_ACCUMULATOR, 0, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x7F, 0x00, 0x00, 0xABCE, 0xFD, 0x35, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    if((result = nesl_test_instruction(0xABCD, 0x46, OPERAND_ZEROPAGE, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABCF, 0xFD, 0x35, 4) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0055) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x56, OPERAND_ZEROPAGE_X, 0x0055, 0, 0xFF)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0x00, 0xABCF, 0xFD, 0x35, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0058) == 0x7F))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem store instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_store(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0x8D, OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0xAB, 0x00, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x9D, OPERAND_ABSOLUTE_X, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0xAB, 0x03, 0x00, 0xABD0, 0xFD, 0x34, 4) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4458) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x99, OPERAND_ABSOLUTE_Y, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0xAB, 0x00, 0x03, 0xABD0, 0xFD, 0x34, 4) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4458) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x81, OPERAND_INDIRECT_X, 0x4455, 0x0032, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0035) == 0x55)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0036) == 0x44)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x91, OPERAND_INDIRECT_Y, 0x4455, 0x0032, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0xAB, 0x00, 0x03, 0xABCF, 0xFD, 0x34, 5) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0032) == 0x55)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0033) == 0x44)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4458) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0x85, OPERAND_ZEROPAGE, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0xAB, 0x00, 0x00, 0xABCF, 0xFD, 0x34, 2) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0055) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x95, OPERAND_ZEROPAGE_X, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0xAB, 0x03, 0x00, 0xABCF, 0xFD, 0x34, 3) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0058) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0x8E, OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0xAB, 0x00, 0xABD0, 0xFD, 0x34, 3) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0x86, OPERAND_ZEROPAGE, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0xAB, 0x00, 0xABCF, 0xFD, 0x34, 2) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0055) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.x.low = 0xAB;
    g_test.processor.state.index.y.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x96, OPERAND_ZEROPAGE_Y, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0xAB, 0x03, 0xABCF, 0xFD, 0x34, 3) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0058) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0x8C, OPERAND_ABSOLUTE, 0x4455, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0xAB, 0xABD0, 0xFD, 0x34, 3) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x4455) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;

    if((result = nesl_test_instruction(0xABCD, 0x84, OPERAND_ZEROPAGE, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0xAB, 0xABCF, 0xFD, 0x34, 2) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0055) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.index.y.low = 0xAB;
    g_test.processor.state.index.x.low = 0x03;

    if((result = nesl_test_instruction(0xABCD, 0x94, OPERAND_ZEROPAGE_X, 0x0055, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT((nesl_test_validate(0x00, 0x03, 0xAB, 0xABCF, 0xFD, 0x34, 3) == true)
            && (nesl_bus_read(BUS_PROCESSOR, 0x0058) == 0xAB))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem transfer instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_transfer(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;
    g_test.processor.state.index.x.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0xAA, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x36, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0xF0;
    g_test.processor.state.index.x.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0xAA, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0xF0, 0xF0, 0x00, 0xABCE, 0xFD, 0xB4, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;
    g_test.processor.state.index.x.low = 0x00;

    if((result = nesl_test_instruction(0xABCD, 0xAA, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x10, 0x10, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x10;
    g_test.processor.state.index.y.low = 0x00;

    if((result = nesl_test_instruction(0xABCD, 0xA8, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x10, 0x00, 0x10, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0x10;
    g_test.processor.state.index.x.low = 0x00;

    if((result = nesl_test_instruction(0xABCD, 0xBA, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x10, 0x00, 0xABCE, 0x10, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;
    g_test.processor.state.index.x.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0x8A, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x10, 0x10, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0x10;
    g_test.processor.state.index.x.low = 0x00;

    if((result = nesl_test_instruction(0xABCD, 0x9A, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0x00, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0x00;
    g_test.processor.state.index.x.low = 0xF0;

    if((result = nesl_test_instruction(0xABCD, 0x9A, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0xF0, 0x00, 0xABCE, 0xF0, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.stack_pointer.low = 0x00;
    g_test.processor.state.index.x.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0x9A, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x00, 0x10, 0x00, 0xABCE, 0x10, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    g_test.processor.state.accumulator.low = 0x00;
    g_test.processor.state.index.y.low = 0x10;

    if((result = nesl_test_instruction(0xABCD, 0x98, OPERAND_IMPLIED, 0, 0, 0)) == NESL_FAILURE) {
        goto exit;
    }

    if(ASSERT(nesl_test_validate(0x10, 0x00, 0x10, 0xABCE, 0xFD, 0x34, 1) == true)) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem unsupported instruction.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_instruction_unsupported(void)
{
    static const nesl_instruction_t UNSUPPORTED[] = {
        { 0x02, OPERAND_IMPLIED, 2 }, { 0x03, OPERAND_INDIRECT_X, 8 }, { 0x04, OPERAND_ZEROPAGE, 3 }, { 0x07, OPERAND_ZEROPAGE, 5 },
        { 0x0B, OPERAND_IMMEDIATE, 2 }, { 0x0C, OPERAND_ABSOLUTE, 4 }, { 0x0F, OPERAND_ABSOLUTE, 6 }, { 0x12, OPERAND_IMPLIED, 2 },
        { 0x13, OPERAND_INDIRECT_Y, 8 }, { 0x14, OPERAND_ZEROPAGE_X, 4 }, { 0x17, OPERAND_ZEROPAGE_X, 6 }, { 0x1A, OPERAND_IMMEDIATE, 2 },
        { 0x1B, OPERAND_ABSOLUTE_Y, 7 }, { 0x1C, OPERAND_ABSOLUTE_X, 4 }, { 0x1F, OPERAND_ABSOLUTE_X, 7 }, { 0x22, OPERAND_IMPLIED, 2 },
        { 0x23, OPERAND_INDIRECT_X, 8 }, { 0x27, OPERAND_ZEROPAGE, 5 }, { 0x2B, OPERAND_IMMEDIATE, 2 }, { 0x2F, OPERAND_ABSOLUTE, 6 },
        { 0x32, OPERAND_IMPLIED, 2 }, { 0x33, OPERAND_INDIRECT_Y, 8 }, { 0x34, OPERAND_ZEROPAGE_X, 4 }, { 0x37, OPERAND_ZEROPAGE_X, 6 },
        { 0x3A, OPERAND_IMMEDIATE, 2 }, { 0x3B, OPERAND_ABSOLUTE_Y, 7 }, { 0x3C, OPERAND_ABSOLUTE_X, 4 }, { 0x3F, OPERAND_ABSOLUTE_X, 7 },
        { 0x42, OPERAND_IMPLIED, 2 }, { 0x43, OPERAND_INDIRECT_X, 8 }, { 0x44, OPERAND_ZEROPAGE, 3 }, { 0x47, OPERAND_ZEROPAGE, 5 },
        { 0x4B, OPERAND_IMMEDIATE, 2 }, { 0x4F, OPERAND_ABSOLUTE, 6 }, { 0x52, OPERAND_IMPLIED, 2 }, { 0x53, OPERAND_INDIRECT_Y, 8 },
        { 0x54, OPERAND_ZEROPAGE_X, 4 }, { 0x57, OPERAND_ZEROPAGE_X, 6 }, { 0x5A, OPERAND_IMMEDIATE, 2 }, { 0x5B, OPERAND_ABSOLUTE_Y, 7 },
        { 0x5C, OPERAND_ABSOLUTE_X, 4 }, { 0x5F, OPERAND_ABSOLUTE_X, 7 }, { 0x62, OPERAND_IMPLIED, 2 }, { 0x63, OPERAND_INDIRECT_X, 8 },
        { 0x64, OPERAND_ZEROPAGE, 3 }, { 0x67, OPERAND_ZEROPAGE, 5 }, { 0x6B, OPERAND_IMMEDIATE, 2 }, { 0x6F, OPERAND_ABSOLUTE, 6 },
        { 0x72, OPERAND_IMPLIED, 2 }, { 0x73, OPERAND_INDIRECT_Y, 8 }, { 0x74, OPERAND_ZEROPAGE_X, 4 }, { 0x77, OPERAND_ZEROPAGE_X, 6 },
        { 0x7A, OPERAND_IMMEDIATE, 2 }, { 0x7B, OPERAND_ABSOLUTE_Y, 7 }, { 0x7C, OPERAND_ABSOLUTE_X, 4 }, { 0x7F, OPERAND_ABSOLUTE_X, 7 },
        { 0x82, OPERAND_IMMEDIATE, 2 }, { 0x83, OPERAND_INDIRECT_X, 6 }, { 0x87, OPERAND_ZEROPAGE, 3 }, { 0x89, OPERAND_IMMEDIATE, 2 },
        { 0x8B, OPERAND_IMMEDIATE, 2 }, { 0x8F, OPERAND_ABSOLUTE, 4 }, { 0x92, OPERAND_IMPLIED, 2 }, { 0x93, OPERAND_INDIRECT_Y, 6 },
        { 0x97, OPERAND_ZEROPAGE_Y, 4 }, { 0x9B, OPERAND_ABSOLUTE_Y, 5 }, { 0x9C, OPERAND_ABSOLUTE_X, 5 }, { 0x9E, OPERAND_ABSOLUTE_Y, 5 },
        { 0x9F, OPERAND_ABSOLUTE_Y, 5 }, { 0xA3, OPERAND_INDIRECT_X, 6 }, { 0xA7, OPERAND_ZEROPAGE, 3 }, { 0xAB, OPERAND_IMMEDIATE, 2 },
        { 0xAF, OPERAND_ABSOLUTE, 4 }, { 0xB2, OPERAND_IMPLIED, 2 }, { 0xB3, OPERAND_INDIRECT_Y, 5 }, { 0xB7, OPERAND_ZEROPAGE_Y, 4 },
        { 0xBB, OPERAND_ABSOLUTE_Y, 4 }, { 0xBF, OPERAND_ABSOLUTE_Y, 4 }, { 0xC2, OPERAND_IMMEDIATE, 2 }, { 0xC3, OPERAND_INDIRECT_X, 8 },
        { 0xC7, OPERAND_ZEROPAGE, 5 }, { 0xCB, OPERAND_IMMEDIATE, 2 }, { 0xCF, OPERAND_ABSOLUTE, 6 }, { 0xD2, OPERAND_IMPLIED, 2 },
        { 0xD3, OPERAND_INDIRECT_Y, 8 }, { 0xD4, OPERAND_ZEROPAGE_X, 4 }, { 0xD7, OPERAND_ZEROPAGE_X, 6 }, { 0xDA, OPERAND_IMMEDIATE, 2 },
        { 0xDB, OPERAND_ABSOLUTE_Y, 7 }, { 0xDC, OPERAND_ABSOLUTE_X, 4 }, { 0xDF, OPERAND_ABSOLUTE_X, 7 }, { 0xE2, OPERAND_IMMEDIATE, 2 },
        { 0xE3, OPERAND_INDIRECT_X, 8 }, { 0xE7, OPERAND_ZEROPAGE, 5 }, { 0xEB, OPERAND_IMMEDIATE, 2 }, { 0xEF, OPERAND_ABSOLUTE, 6 },
        { 0xF2, OPERAND_IMPLIED, 2 }, { 0xF3, OPERAND_INDIRECT_Y, 8 }, { 0xF4, OPERAND_ZEROPAGE_X, 4 }, { 0xF7, OPERAND_ZEROPAGE_X, 6 },
        { 0xFA, OPERAND_IMMEDIATE, 2 }, { 0xFB, OPERAND_ABSOLUTE_Y, 7 }, { 0xFC, OPERAND_ABSOLUTE_X, 4 }, { 0xFF, OPERAND_ABSOLUTE_X, 7 },
        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < (sizeof(UNSUPPORTED) / sizeof(nesl_instruction_t)); ++index) {
        uint16_t address = 0xABCD;
        const nesl_instruction_t *unsupported = &UNSUPPORTED[index];

        if((result = nesl_test_initialize(address, true)) == NESL_FAILURE) {
            goto exit;
        }

        if((result = nesl_test_instruction(address++, unsupported->type, unsupported->mode, 0, 0, 0)) == NESL_FAILURE) {
            goto exit;
        }

        switch(unsupported->mode) {
            case OPERAND_ABSOLUTE:
            case OPERAND_ABSOLUTE_X:
            case OPERAND_ABSOLUTE_Y:
            case OPERAND_INDIRECT:
                address += 2;
                break;
            case OPERAND_IMMEDIATE:
            case OPERAND_INDIRECT_X:
            case OPERAND_INDIRECT_Y:
            case OPERAND_RELATIVE:
            case OPERAND_ZEROPAGE:
            case OPERAND_ZEROPAGE_X:
            case OPERAND_ZEROPAGE_Y:
                ++address;
                break;
            default:
                break;
        }

        if(ASSERT(nesl_test_validate(0x00, 0x00, 0x00, address, 0xFD, 0x34, unsupported->cycles - 1) == true)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem interrupt.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_interrupt(void)
{
    uint16_t address;
    nesl_error_e result;
    nesl_processor_status_t status = {};

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    nesl_processor_interrupt(&g_test.processor, false);

    if(ASSERT(g_test.processor.interrupt.non_maskable)) {
        result = NESL_FAILURE;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    nesl_processor_interrupt(&g_test.processor, true);

    if(ASSERT(g_test.processor.interrupt.maskable)) {
        result = NESL_FAILURE;
        goto exit;
    }

   if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    address = 0x1234;
    g_test.processor.cycle = 0;
    nesl_bus_write(BUS_PROCESSOR, 0xFFFA, address);
    nesl_bus_write(BUS_PROCESSOR, 0xFFFB, address >> 8);
    nesl_processor_interrupt(&g_test.processor, false);
    nesl_processor_cycle(&g_test.processor, 0);

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, address, 0xFA, 0x34, 6) == true)
            && (g_test.processor.interrupt.non_maskable == false)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 1) == g_test.processor.state.status.raw)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 2) == 0xCD)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 3) == 0xAB))) {
        result = NESL_SUCCESS;
        goto exit;
    }

    g_test.processor.cycle = 0;
    g_test.processor.state.status.interrupt_disable = false;
    status.raw = g_test.processor.state.status.raw;
    nesl_processor_interrupt(&g_test.processor, false);
    nesl_processor_cycle(&g_test.processor, 0);

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, address, 0xF7, 0x34, 6) == true)
            && (g_test.processor.interrupt.non_maskable == false)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 1) == status.raw)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 2) == 0x34)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 3) == 0x12))) {
        result = NESL_SUCCESS;
        goto exit;
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    address = 0x5678;
    g_test.processor.cycle = 0;
    nesl_bus_write(BUS_PROCESSOR, 0xABCD, 0xEA);
    nesl_bus_write(BUS_PROCESSOR, 0xFFFE, address);
    nesl_bus_write(BUS_PROCESSOR, 0xFFFF, address >> 8);
    nesl_processor_interrupt(&g_test.processor, true);
    nesl_processor_cycle(&g_test.processor, 0);

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0xABCE, 0xFD, 0x34, 1) == true)
            && (g_test.processor.interrupt.maskable == true))) {
        result = NESL_SUCCESS;
        goto exit;
    }

    g_test.processor.cycle = 0;
    g_test.processor.state.status.interrupt_disable = false;
    status.raw = g_test.processor.state.status.raw;
    nesl_processor_interrupt(&g_test.processor, true);
    nesl_processor_cycle(&g_test.processor, 0);

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, address, 0xFA, 0x34, 6) == true)
            && (g_test.processor.interrupt.maskable == false)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 1) == status.raw)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 2) == 0xCE)
            && (nesl_bus_read(BUS_PROCESSOR, (0x0100 | g_test.processor.state.stack_pointer.low) + 3) == 0xAB))) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem read.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_read(void)
{
    uint8_t data = 0;
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    for(uint16_t address = 0x0000; address < 0x1FFF; ++address, ++data) {
        g_test.processor.ram[address & 0x07FF] = data;

        if(ASSERT(nesl_processor_read(&g_test.processor, address) == data)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem reset.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_reset(void)
{
    nesl_error_e result;
    uint16_t address = 0xDCBA;

    if((result = nesl_test_initialize(0xABCD, false)) == NESL_FAILURE) {
        goto exit;
    }

    nesl_bus_write(BUS_PROCESSOR, 0xFFFC, address);
    nesl_bus_write(BUS_PROCESSOR, 0xFFFD, address >> 8);
    nesl_processor_reset(&g_test.processor);

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, address, 0xFD, 0x34, 7) == true)
            && (g_test.processor.interrupt.raw == 0)
            && (g_test.processor.transfer.destination.word == 0)
            && (g_test.processor.transfer.source.word == 0))) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem transfer.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_transfer(void)
{
    uint64_t cycle;
    uint16_t address;
    nesl_error_e result;
    uint8_t data = 0, page = 0xAB;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    for(address = (page << 8); address <= ((page << 8) | 0xFF); ++address) {
        nesl_bus_write(BUS_PROCESSOR, address, data++);
    }

    cycle = 0;
    g_test.processor.cycle = 0;
    nesl_processor_write(&g_test.processor, 0x4014, page);

    if(ASSERT((g_test.processor.interrupt.transfer == true)
            && (g_test.processor.interrupt.transfer_sync == true)
            && (g_test.processor.transfer.source.word == (page << 8))
            && (g_test.processor.transfer.destination.word == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_processor_cycle(&g_test.processor, cycle);
    cycle += 3;
    nesl_processor_cycle(&g_test.processor, cycle);
    cycle += 3;

    for(address = 0; address <= 0xFF; ++address) {
        nesl_processor_cycle(&g_test.processor, cycle);
        cycle += 3;

        nesl_processor_cycle(&g_test.processor, cycle);
        cycle += 3;

        if(ASSERT((g_test.processor.interrupt.transfer == (address < 0xFF))
                && (g_test.processor.interrupt.transfer_sync == false)
                && (g_test.processor.transfer.source.word == ((address < 0xFF) ? ((page << 8) + address + 1) : 0))
                && (g_test.processor.transfer.destination.word == ((address < 0xFF) ? address + 1 : 0))
                && (nesl_bus_read(BUS_VIDEO_OAM, address) == nesl_bus_read(BUS_PROCESSOR, (page << 8) + address)))) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    for(address = (page << 8); address <= ((page << 8) | 0xFF); ++address) {
        nesl_bus_write(BUS_PROCESSOR, address, data++);
    }

    cycle = 3;

    g_test.processor.cycle = 0;
    nesl_processor_write(&g_test.processor, 0x4014, page);

    if(ASSERT((g_test.processor.interrupt.transfer == true)
            && (g_test.processor.interrupt.transfer_sync == true)
            && (g_test.processor.transfer.source.word == (page << 8))
            && (g_test.processor.transfer.destination.word == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

    nesl_processor_cycle(&g_test.processor, cycle);
    cycle += 3;

    for(address = 0; address <= 0xFF; ++address) {
        nesl_processor_cycle(&g_test.processor, cycle);
        cycle += 3;

        nesl_processor_cycle(&g_test.processor, cycle);
        cycle += 3;

        if(ASSERT((g_test.processor.interrupt.transfer == (address < 0xFF))
                && (g_test.processor.interrupt.transfer_sync == false)
                && (g_test.processor.transfer.source.word == ((address < 0xFF) ? ((page << 8) + address + 1) : 0))
                && (g_test.processor.transfer.destination.word == ((address < 0xFF) ? address + 1 : 0))
                && (nesl_bus_read(BUS_VIDEO_OAM, address) == nesl_bus_read(BUS_PROCESSOR, (page << 8) + address)))) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem uninitialization.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_uninitialize(void)
{
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    nesl_processor_uninitialize(&g_test.processor);

    if(ASSERT((nesl_test_validate(0x00, 0x00, 0x00, 0x0000, 0x00, 0x00, 0) == true)
            && (g_test.processor.interrupt.raw == 0)
            && (g_test.processor.transfer.destination.word == 0)
            && (g_test.processor.transfer.source.word == 0))) {
        result = NESL_SUCCESS;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

/**
 * @brief Test processor subsystem write.
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
static nesl_error_e nesl_test_processor_write(void)
{
    uint8_t data = 0;
    nesl_error_e result;

    if((result = nesl_test_initialize(0xABCD, true)) == NESL_FAILURE) {
        goto exit;
    }

    for(uint16_t address = 0x0000; address < 0x1FFF; ++address, ++data) {
        nesl_processor_write(&g_test.processor, address, data);

        if(ASSERT(g_test.processor.ram[address & 0x07FF] == data)) {
            result = NESL_FAILURE;
            goto exit;
        }
    }

    data = 0xAB;
    nesl_processor_write(&g_test.processor, 0x4014, data);

    if(ASSERT((g_test.processor.interrupt.transfer == true)
            && (g_test.processor.interrupt.transfer_sync == true)
            && (g_test.processor.transfer.source.word == (data << 8))
            && (g_test.processor.transfer.destination.word == 0))) {
        result = NESL_FAILURE;
        goto exit;
    }

exit:
    TEST_RESULT(result);

    return result;
}

int main(void)
{
    static const test TEST[] = {
        nesl_test_processor_cycle, nesl_test_processor_initialize, nesl_test_processor_instruction_arithmetic, nesl_test_processor_instruction_bit,
        nesl_test_processor_instruction_branch, nesl_test_processor_instruction_breakpoint, nesl_test_processor_instruction_clear, nesl_test_processor_instruction_compare,
        nesl_test_processor_instruction_decrement, nesl_test_processor_instruction_increment, nesl_test_processor_instruction_jump, nesl_test_processor_instruction_load,
        nesl_test_processor_instruction_logical, nesl_test_processor_instruction_no_operation, nesl_test_processor_instruction_pull, nesl_test_processor_instruction_push,
        nesl_test_processor_instruction_return, nesl_test_processor_instruction_rotate, nesl_test_processor_instruction_set, nesl_test_processor_instruction_shift,
        nesl_test_processor_instruction_store, nesl_test_processor_instruction_transfer, nesl_test_processor_instruction_unsupported, nesl_test_processor_interrupt,
        nesl_test_processor_read, nesl_test_processor_reset, nesl_test_processor_transfer, nesl_test_processor_uninitialize,
        nesl_test_processor_write,
        };

    nesl_error_e result = NESL_SUCCESS;

    for(int index = 0; index < TEST_COUNT(TEST); ++index) {

        if(TEST[index]() == NESL_FAILURE) {
            result = NESL_FAILURE;
        }
    }

    return (int)result;
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
