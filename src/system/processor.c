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

typedef struct {
    int type;
    int mode;
    uint8_t cycles;
} nesl_instruction_t;

typedef struct {
    nesl_register_t data;
    nesl_register_t effective;
    nesl_register_t indirect;
    bool page_cross;
} nesl_operand_t;

typedef void (*nesl_processor_address)(nesl_processor_t *processor, nesl_operand_t *operand);

typedef void (*nesl_processor_execute)(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand);

typedef void (*nesl_processor_operation)(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right);

static const nesl_instruction_t INSTRUCTION[] = {
    { NESL_INSTRUCTION_BRK, NESL_ADDRESS_IMPLIED, 7 },
    { NESL_INSTRUCTION_ORA, NESL_ADDRESS_INDIRECT_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_X, 8 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_ORA, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_ASL, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_PHP, NESL_ADDRESS_IMPLIED, 3 },
    { NESL_INSTRUCTION_ORA, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_ASL, NESL_ADDRESS_ACCUMULATOR, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_ORA, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_ASL, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_BPL, NESL_ADDRESS_RELATIVE, 2 },
    { NESL_INSTRUCTION_ORA, NESL_ADDRESS_INDIRECT_Y, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_Y, 8 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_ORA, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_ASL, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_CLC, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_ORA, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_ORA, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_ASL, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { NESL_INSTRUCTION_JSR, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_AND, NESL_ADDRESS_INDIRECT_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_X, 8 },
    { NESL_INSTRUCTION_BIT, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_AND, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_ROL, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_PLP, NESL_ADDRESS_IMPLIED, 4 },
    { NESL_INSTRUCTION_AND, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_ROL, NESL_ADDRESS_ACCUMULATOR, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_BIT, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_AND, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_ROL, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_BMI, NESL_ADDRESS_RELATIVE, 2 },
    { NESL_INSTRUCTION_AND, NESL_ADDRESS_INDIRECT_Y, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_Y, 8 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_AND, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_ROL, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_SEC, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_AND, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_AND, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_ROL, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { NESL_INSTRUCTION_RTI, NESL_ADDRESS_IMPLIED, 6 },
    { NESL_INSTRUCTION_EOR, NESL_ADDRESS_INDIRECT_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_X, 8 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_EOR, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_LSR, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_PHA, NESL_ADDRESS_IMPLIED, 3 },
    { NESL_INSTRUCTION_EOR, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_LSR, NESL_ADDRESS_ACCUMULATOR, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_JMP, NESL_ADDRESS_ABSOLUTE, 3 },
    { NESL_INSTRUCTION_EOR, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_LSR, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_BVC, NESL_ADDRESS_RELATIVE, 2 },
    { NESL_INSTRUCTION_EOR, NESL_ADDRESS_INDIRECT_Y, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_Y, 8 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_EOR, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_LSR, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_CLI, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_EOR, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_EOR, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_LSR, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { NESL_INSTRUCTION_RTS, NESL_ADDRESS_IMPLIED, 6 },
    { NESL_INSTRUCTION_ADC, NESL_ADDRESS_INDIRECT_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_X, 8 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_ADC, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_ROR, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_PLA, NESL_ADDRESS_IMPLIED, 4 },
    { NESL_INSTRUCTION_ADC, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_ROR, NESL_ADDRESS_ACCUMULATOR, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_JMP, NESL_ADDRESS_INDIRECT, 5 },
    { NESL_INSTRUCTION_ADC, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_ROR, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_BVS, NESL_ADDRESS_RELATIVE, 2 },
    { NESL_INSTRUCTION_ADC, NESL_ADDRESS_INDIRECT_Y, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_Y, 8 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_ADC, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_ROR, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_SEI, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_ADC, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_ADC, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_ROR, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_STA, NESL_ADDRESS_INDIRECT_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_X, 6 },
    { NESL_INSTRUCTION_STY, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_STA, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_STX, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_DEY, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_TXA, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_STY, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_STA, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_STX, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_BCC, NESL_ADDRESS_RELATIVE, 2 },
    { NESL_INSTRUCTION_STA, NESL_ADDRESS_INDIRECT_Y, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_Y, 6 },
    { NESL_INSTRUCTION_STY, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_STA, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_STX, NESL_ADDRESS_ZEROPAGE_Y, 4 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_Y, 4 },
    { NESL_INSTRUCTION_TYA, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_STA, NESL_ADDRESS_ABSOLUTE_Y, 5 },
    { NESL_INSTRUCTION_TXS, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_Y, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 5 },
    { NESL_INSTRUCTION_STA, NESL_ADDRESS_ABSOLUTE_X, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_Y, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_Y, 5 },
    { NESL_INSTRUCTION_LDY, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_LDA, NESL_ADDRESS_INDIRECT_X, 6 },
    { NESL_INSTRUCTION_LDX, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_X, 6 },
    { NESL_INSTRUCTION_LDY, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_LDA, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_LDX, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_TAY, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_LDA, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_TAX, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_LDY, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_LDA, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_LDX, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_BCS, NESL_ADDRESS_RELATIVE, 2 },
    { NESL_INSTRUCTION_LDA, NESL_ADDRESS_INDIRECT_Y, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_Y, 5 },
    { NESL_INSTRUCTION_LDY, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_LDA, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_LDX, NESL_ADDRESS_ZEROPAGE_Y, 4 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_Y, 4 },
    { NESL_INSTRUCTION_CLV, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_LDA, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { NESL_INSTRUCTION_TSX, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { NESL_INSTRUCTION_LDY, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_LDA, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_LDX, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { NESL_INSTRUCTION_CPY, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_CMP, NESL_ADDRESS_INDIRECT_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_X, 8 },
    { NESL_INSTRUCTION_CPY, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_CMP, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_DEC, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_INY, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_CMP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_DEX, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_CPY, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_CMP, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_DEC, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_BNE, NESL_ADDRESS_RELATIVE, 2 },
    { NESL_INSTRUCTION_CMP, NESL_ADDRESS_INDIRECT_Y, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_Y, 8 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_CMP, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_DEC, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_CLD, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_CMP, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_CMP, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_DEC, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { NESL_INSTRUCTION_CPX, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_SBC, NESL_ADDRESS_INDIRECT_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_X, 8 },
    { NESL_INSTRUCTION_CPX, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_SBC, NESL_ADDRESS_ZEROPAGE, 3 },
    { NESL_INSTRUCTION_INC, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE, 5 },
    { NESL_INSTRUCTION_INX, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_SBC, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_CPX, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_SBC, NESL_ADDRESS_ABSOLUTE, 4 },
    { NESL_INSTRUCTION_INC, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE, 6 },
    { NESL_INSTRUCTION_BEQ, NESL_ADDRESS_RELATIVE, 2 },
    { NESL_INSTRUCTION_SBC, NESL_ADDRESS_INDIRECT_Y, 5 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_INDIRECT_Y, 8 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_SBC, NESL_ADDRESS_ZEROPAGE_X, 4 },
    { NESL_INSTRUCTION_INC, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ZEROPAGE_X, 6 },
    { NESL_INSTRUCTION_SED, NESL_ADDRESS_IMPLIED, 2 },
    { NESL_INSTRUCTION_SBC, NESL_ADDRESS_ABSOLUTE_Y, 4 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_IMMEDIATE, 2 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_Y, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_SBC, NESL_ADDRESS_ABSOLUTE_X, 4 },
    { NESL_INSTRUCTION_INC, NESL_ADDRESS_ABSOLUTE_X, 7 },
    { NESL_INSTRUCTION_NOP, NESL_ADDRESS_ABSOLUTE_X, 7 },
    };

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static uint8_t nesl_processor_fetch(nesl_processor_t *processor)
{
    return nesl_bus_read(NESL_BUS_PROCESSOR, processor->state.program_counter.word++);
}

static uint16_t nesl_processor_fetch_word(nesl_processor_t *processor)
{
    return nesl_processor_fetch(processor) | (nesl_processor_fetch(processor) << 8);
}

static uint8_t nesl_processor_pull(nesl_processor_t *processor)
{
    return nesl_bus_read(NESL_BUS_PROCESSOR, 0x0100 | ++processor->state.stack_pointer.low);
}

static uint16_t nesl_processor_pull_word(nesl_processor_t *processor)
{
    return nesl_processor_pull(processor) | (nesl_processor_pull(processor) << 8);
}

static void nesl_processor_push(nesl_processor_t *processor, uint8_t data)
{
    nesl_bus_write(NESL_BUS_PROCESSOR, 0x0100 | processor->state.stack_pointer.low--, data);
}

static void nesl_processor_push_word(nesl_processor_t *processor, uint16_t data)
{
    nesl_processor_push(processor, data >> 8);
    nesl_processor_push(processor, data);
}

static uint16_t nesl_processor_read_word(nesl_processor_t *processor, uint16_t address)
{
    return nesl_bus_read(NESL_BUS_PROCESSOR, address) | (nesl_bus_read(NESL_BUS_PROCESSOR, address + 1) << 8);
}

static void nesl_processor_address_absolute(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch_word(processor);
    operand->effective.word = operand->data.word;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void nesl_processor_address_absolute_x(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch_word(processor);
    operand->effective.word = operand->data.word + processor->state.index.x.low;
    operand->indirect.word = 0;
    operand->page_cross = (operand->effective.high != operand->data.high);
}

static void nesl_processor_address_absolute_y(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch_word(processor);
    operand->effective.word = operand->data.word + processor->state.index.y.low;
    operand->indirect.word = 0;
    operand->page_cross = (operand->effective.high != operand->data.high);
}

static void nesl_processor_address_accumulator(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = processor->state.accumulator.low;
    operand->effective.word = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void nesl_processor_address_immediate(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->effective.word = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void nesl_processor_address_implied(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = 0;
    operand->effective.word = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void nesl_processor_address_indirect(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch_word(processor);
    operand->indirect.word = operand->data.word;

    if(operand->indirect.low == 0xFF) {
        operand->effective.low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->indirect.word);
        operand->effective.high = nesl_bus_read(NESL_BUS_PROCESSOR, operand->indirect.word & 0xFF00);
    } else {
        operand->effective.word = nesl_processor_read_word(processor, operand->indirect.word);
    }

    operand->page_cross = false;
}

static void nesl_processor_address_indirect_x(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->indirect.low = (operand->data.word + processor->state.index.x.low);
    operand->indirect.high = 0;
    operand->effective.low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->indirect.low);
    operand->effective.high = nesl_bus_read(NESL_BUS_PROCESSOR, (operand->indirect.low + 1) & 0xFF);
    operand->page_cross = false;
}

static void nesl_processor_address_indirect_y(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->indirect.low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->data.low);
    operand->indirect.high = nesl_bus_read(NESL_BUS_PROCESSOR, (operand->data.low + 1) & 0xFF);
    operand->effective.word = operand->indirect.word + processor->state.index.y.low;
    operand->page_cross = (operand->effective.high != operand->indirect.high);
}

static void nesl_processor_address_relative(nesl_processor_t *processor, nesl_operand_t *operand)

{
    operand->data.word = nesl_processor_fetch(processor);

    if(operand->data.bit_7) {
        operand->data.high = 0xFF;
    }

    operand->effective.word = processor->state.program_counter.word + operand->data.word;
    operand->indirect.word = 0;
    operand->page_cross = (operand->effective.high != processor->state.program_counter.high);
}

static void nesl_processor_address_zeropage(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->effective.word = operand->data.word;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void nesl_processor_address_zeropage_x(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->effective.low = operand->data.low + processor->state.index.x.low;
    operand->effective.high = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void nesl_processor_address_zeropage_y(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->effective.low = operand->data.low + processor->state.index.y.low;
    operand->effective.high = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static const nesl_processor_address ADDRESS[] = {
    nesl_processor_address_absolute,
    nesl_processor_address_absolute_x,
    nesl_processor_address_absolute_y,
    nesl_processor_address_accumulator,
    nesl_processor_address_immediate,
    nesl_processor_address_implied,
    nesl_processor_address_indirect,
    nesl_processor_address_indirect_x,
    nesl_processor_address_indirect_y,
    nesl_processor_address_relative,
    nesl_processor_address_zeropage,
    nesl_processor_address_zeropage_x,
    nesl_processor_address_zeropage_y,
    };

static void nesl_processor_operation_add_carry(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    nesl_register_t data = {};

    data.word = left->low + right->low + (processor->state.status.carry ? 1 : 0);
    processor->state.status.carry = data.high > 0;
    processor->state.status.overflow = !(left->bit_7 ^ right->bit_7) && (left->bit_7 ^ data.bit_7);
    left->low = data.low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void nesl_processor_operation_compare(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    nesl_register_t data = {};

    data.low = right->low;
    processor->state.status.carry = (right->low >= left->low);
    data.low -= left->low;
    left->low = data.low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void nesl_processor_operation_decrement(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    --left->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void nesl_processor_operation_increment(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    ++left->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void nesl_processor_operation_logical_and(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    left->low &= right->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void nesl_processor_operation_logical_exclusive_or(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    left->low ^= right->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void nesl_processor_operation_logical_or(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    left->low |= right->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void nesl_processor_operation_rotate_left(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    bool carry = processor->state.status.carry;

    processor->state.status.carry = left->bit_7;
    left->low <<= 1;
    left->bit_0 = carry;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void nesl_processor_operation_rotate_right(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    bool carry = processor->state.status.carry;

    processor->state.status.carry = left->bit_0;
    left->low >>= 1;
    left->bit_7 = carry;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void nesl_processor_operation_shift_left(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    processor->state.status.carry = left->bit_7;
    left->low <<= 1;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void nesl_processor_operation_shift_right(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    processor->state.status.carry = left->bit_0;
    left->low >>= 1;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void nesl_processor_operation_transfer(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{

    if(left != &processor->state.stack_pointer) {
        processor->state.status.negative = right->bit_7;
        processor->state.status.zero = !right->low;
    }

    left->low = right->low;
}

static const nesl_processor_operation OPERATION[] = {
    nesl_processor_operation_add_carry,
    nesl_processor_operation_compare,
    nesl_processor_operation_decrement,
    nesl_processor_operation_increment,
    nesl_processor_operation_logical_and,
    nesl_processor_operation_logical_exclusive_or,
    nesl_processor_operation_logical_or,
    nesl_processor_operation_rotate_left,
    nesl_processor_operation_rotate_right,
    nesl_processor_operation_shift_left,
    nesl_processor_operation_shift_right,
    nesl_processor_operation_transfer,
    };

static void nesl_processor_execute_arithmetic(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_ADDRESS_IMMEDIATE) {
        data.low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->effective.word);
    }

    if(instruction->type == NESL_INSTRUCTION_SBC) {
        data.low = ~data.low;
    }

    OPERATION[NESL_OPERATION_ADD_CARRY](processor, &processor->state.accumulator, &data);

    if(operand->page_cross) {
        ++processor->cycle;
    }
}

static void nesl_processor_execute_bit(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->effective.word) };

    processor->state.status.negative = data.bit_7;
    processor->state.status.overflow = data.bit_6;
    processor->state.status.zero = !(processor->state.accumulator.low & data.low);
}

static void nesl_processor_execute_branch(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    bool branch = false;

    switch(instruction->type) {
        case NESL_INSTRUCTION_BCC:
            branch = !processor->state.status.carry;
            break;
        case NESL_INSTRUCTION_BCS:
            branch = processor->state.status.carry;
            break;
        case NESL_INSTRUCTION_BEQ:
            branch = processor->state.status.zero;
            break;
        case NESL_INSTRUCTION_BMI:
            branch = processor->state.status.negative;
            break;
        case NESL_INSTRUCTION_BNE:
            branch = !processor->state.status.zero;
            break;
        case NESL_INSTRUCTION_BPL:
            branch = !processor->state.status.negative;
            break;
        case NESL_INSTRUCTION_BVC:
            branch = !processor->state.status.overflow;
            break;
        case NESL_INSTRUCTION_BVS:
            branch = processor->state.status.overflow;
            break;
        default:
            break;
    }

    if(branch) {
        processor->state.program_counter.word = operand->effective.word;

        if(operand->page_cross) {
            ++processor->cycle;
        }

        ++processor->cycle;
    }
}

static void nesl_processor_execute_breakpoint(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_processor_push_word(processor, processor->state.program_counter.word + 1);
    processor->state.status.breakpoint = true;
    nesl_processor_push(processor, processor->state.status.raw);
    processor->state.status.interrupt_disable = true;
    processor->state.program_counter.word = nesl_processor_read_word(processor, 0xFFFE);
}

static void nesl_processor_execute_clear(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case NESL_INSTRUCTION_CLC:
            processor->state.status.carry = false;
            break;
        case NESL_INSTRUCTION_CLD:
            processor->state.status.decimal = false;
            break;
        case NESL_INSTRUCTION_CLI:
            processor->state.status.interrupt_disable = false;
            break;
        case NESL_INSTRUCTION_CLV:
            processor->state.status.overflow = false;
            break;
        default:
            break;
    }
}

static void nesl_processor_execute_compare(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_ADDRESS_IMMEDIATE) {
        data.low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case NESL_INSTRUCTION_CMP:
            OPERATION[NESL_OPERATION_COMPARE](processor, &data, &processor->state.accumulator);
            break;
        case NESL_INSTRUCTION_CPX:
            OPERATION[NESL_OPERATION_COMPARE](processor, &data, &processor->state.index.x);
            break;
        case NESL_INSTRUCTION_CPY:
            OPERATION[NESL_OPERATION_COMPARE](processor, &data, &processor->state.index.y);
            break;
        default:
            break;
    }

    if(operand->page_cross) {
        ++processor->cycle;
    }
}

static void nesl_processor_execute_decrement(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = {};

    switch(instruction->type) {
        case NESL_INSTRUCTION_DEC:
            data.low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->effective.word);
            OPERATION[NESL_OPERATION_DECREMENT](processor, &data, NULL);
            nesl_bus_write(NESL_BUS_PROCESSOR, operand->effective.word, data.low);
            break;
        case NESL_INSTRUCTION_DEX:
            OPERATION[NESL_OPERATION_DECREMENT](processor, &processor->state.index.x, NULL);
            break;
        case NESL_INSTRUCTION_DEY:
            OPERATION[NESL_OPERATION_DECREMENT](processor, &processor->state.index.y, NULL);
            break;
        default:
            break;
    }
}

static void nesl_processor_execute_increment(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = {};

    switch(instruction->type) {
        case NESL_INSTRUCTION_INC:
            data.low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->effective.word);
            OPERATION[NESL_OPERATION_INCREMENT](processor, &data, NULL);
            nesl_bus_write(NESL_BUS_PROCESSOR, operand->effective.word, data.low);
            break;
        case NESL_INSTRUCTION_INX:
            OPERATION[NESL_OPERATION_INCREMENT](processor, &processor->state.index.x, NULL);
            break;
        case NESL_INSTRUCTION_INY:
            OPERATION[NESL_OPERATION_INCREMENT](processor, &processor->state.index.y, NULL);
            break;
        default:
            break;
    }
}

static void nesl_processor_execute_jump(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    if(instruction->type == NESL_INSTRUCTION_JSR) {
        nesl_processor_push_word(processor, processor->state.program_counter.word - 1);
    }

    processor->state.program_counter.word = operand->effective.word;
}

static void nesl_processor_execute_load(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_ADDRESS_IMMEDIATE) {
        data.low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->effective.word);
    }

    processor->state.status.negative = data.bit_7;
    processor->state.status.zero = !data.low;

    switch(instruction->type) {
        case NESL_INSTRUCTION_LDA:
            processor->state.accumulator.low = data.low;
            break;
        case NESL_INSTRUCTION_LDX:
            processor->state.index.x.low = data.low;
            break;
        case NESL_INSTRUCTION_LDY:
            processor->state.index.y.low = data.low;
            break;
        default:
            break;
    }

    if(operand->page_cross) {
        ++processor->cycle;
    }
}

static void nesl_processor_execute_logical(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_ADDRESS_IMMEDIATE) {
        data.low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case NESL_INSTRUCTION_AND:
            OPERATION[NESL_OPERATION_LOGICAL_AND](processor, &processor->state.accumulator, &data);
            break;
        case NESL_INSTRUCTION_EOR:
            OPERATION[NESL_OPERATION_LOGICAL_EXCLUSIVE_OR](processor, &processor->state.accumulator, &data);
            break;
        case NESL_INSTRUCTION_ORA:
            OPERATION[NESL_OPERATION_LOGICAL_OR](processor, &processor->state.accumulator, &data);
            break;
        default:
            break;
    }

    if(operand->page_cross) {
        ++processor->cycle;
    }
}

static void nesl_processor_execute_no_operation(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    return;
}

static void nesl_processor_execute_pull(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case NESL_INSTRUCTION_PLA:
            processor->state.accumulator.low = nesl_processor_pull(processor);
            processor->state.status.negative = processor->state.accumulator.bit_7;
            processor->state.status.zero = !processor->state.accumulator.low;
            break;
        case NESL_INSTRUCTION_PLP:
            processor->state.status.raw = nesl_processor_pull(processor);
            processor->state.status.unused = true;
            break;
        default:
            break;
    }
}

static void nesl_processor_execute_push(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case NESL_INSTRUCTION_PHA:
            nesl_processor_push(processor, processor->state.accumulator.low);
            break;
        case NESL_INSTRUCTION_PHP:
            nesl_processor_push(processor, processor->state.status.raw);
            break;
        default:
            break;
    }
}

static void nesl_processor_execute_return(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case NESL_INSTRUCTION_RTI:
            processor->state.status.raw = nesl_processor_pull(processor);
            processor->state.status.unused = true;
            processor->state.program_counter.word = nesl_processor_pull_word(processor);
            break;
        case NESL_INSTRUCTION_RTS:
            processor->state.program_counter.word = nesl_processor_pull_word(processor) + 1;
            break;
        default:
            break;
    }
}

static void nesl_processor_execute_rotate(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_ADDRESS_ACCUMULATOR) {
        data.low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case NESL_INSTRUCTION_ROL:
            OPERATION[NESL_OPERATION_ROTATE_LEFT](processor, &data, NULL);
            break;
        case NESL_INSTRUCTION_ROR:
            OPERATION[NESL_OPERATION_ROTATE_RIGHT](processor, &data, NULL);
            break;
        default:
            break;
    }

    if(instruction->mode != NESL_ADDRESS_ACCUMULATOR) {
        nesl_bus_write(NESL_BUS_PROCESSOR, operand->effective.word, data.low);
    } else {
        processor->state.accumulator.low = data.low;
    }
}

static void nesl_processor_execute_set(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case NESL_INSTRUCTION_SEC:
            processor->state.status.carry = true;
            break;
        case NESL_INSTRUCTION_SED:
            processor->state.status.decimal = true;
            break;
        case NESL_INSTRUCTION_SEI:
            processor->state.status.interrupt_disable = true;
            break;
        default:
            break;
    }
}

static void nesl_processor_execute_shift(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_ADDRESS_ACCUMULATOR) {
        data.low = nesl_bus_read(NESL_BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case NESL_INSTRUCTION_ASL:
            OPERATION[NESL_OPERATION_SHIFT_LEFT](processor, &data, NULL);
            break;
        case NESL_INSTRUCTION_LSR:
            OPERATION[NESL_OPERATION_SHIFT_RIGHT](processor, &data, NULL);
            break;
        default:
            break;
    }

    if(instruction->mode != NESL_ADDRESS_ACCUMULATOR) {
        nesl_bus_write(NESL_BUS_PROCESSOR, operand->effective.word, data.low);
    } else {
        processor->state.accumulator.low = data.low;
    }
}

static void nesl_processor_execute_store(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = {};

    switch(instruction->type) {
        case NESL_INSTRUCTION_STA:
            data.low = processor->state.accumulator.low;
            break;
        case NESL_INSTRUCTION_STX:
            data.low = processor->state.index.x.low;
            break;
        case NESL_INSTRUCTION_STY:
            data.low = processor->state.index.y.low;
            break;
        default:
            break;
    }

    nesl_bus_write(NESL_BUS_PROCESSOR, operand->effective.word, data.low);
}

static void nesl_processor_execute_transfer(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case NESL_INSTRUCTION_TAX:
            OPERATION[NESL_OPERATION_TRANSFER](processor, &processor->state.index.x, &processor->state.accumulator);
            break;
        case NESL_INSTRUCTION_TAY:
            OPERATION[NESL_OPERATION_TRANSFER](processor, &processor->state.index.y, &processor->state.accumulator);
            break;
        case NESL_INSTRUCTION_TSX:
            OPERATION[NESL_OPERATION_TRANSFER](processor, &processor->state.index.x, &processor->state.stack_pointer);
            break;
        case NESL_INSTRUCTION_TXA:
            OPERATION[NESL_OPERATION_TRANSFER](processor, &processor->state.accumulator, &processor->state.index.x);
            break;
        case NESL_INSTRUCTION_TXS:
            OPERATION[NESL_OPERATION_TRANSFER](processor, &processor->state.stack_pointer, &processor->state.index.x);
            break;
        case NESL_INSTRUCTION_TYA:
            OPERATION[NESL_OPERATION_TRANSFER](processor, &processor->state.accumulator, &processor->state.index.y);
            break;
        default:
            break;
    }
}

static const nesl_processor_execute EXECUTE[] = {
    nesl_processor_execute_arithmetic,
    nesl_processor_execute_logical,
    nesl_processor_execute_shift,
    nesl_processor_execute_branch,
    nesl_processor_execute_branch,
    nesl_processor_execute_branch,
    nesl_processor_execute_bit,
    nesl_processor_execute_branch,
    nesl_processor_execute_branch,
    nesl_processor_execute_branch,
    nesl_processor_execute_breakpoint,
    nesl_processor_execute_branch,
    nesl_processor_execute_branch,
    nesl_processor_execute_clear,
    nesl_processor_execute_clear,
    nesl_processor_execute_clear,
    nesl_processor_execute_clear,
    nesl_processor_execute_compare,
    nesl_processor_execute_compare,
    nesl_processor_execute_compare,
    nesl_processor_execute_decrement,
    nesl_processor_execute_decrement,
    nesl_processor_execute_decrement,
    nesl_processor_execute_logical,
    nesl_processor_execute_increment,
    nesl_processor_execute_increment,
    nesl_processor_execute_increment,
    nesl_processor_execute_jump,
    nesl_processor_execute_jump,
    nesl_processor_execute_load,
    nesl_processor_execute_load,
    nesl_processor_execute_load,
    nesl_processor_execute_shift,
    nesl_processor_execute_no_operation,
    nesl_processor_execute_logical,
    nesl_processor_execute_push,
    nesl_processor_execute_push,
    nesl_processor_execute_pull,
    nesl_processor_execute_pull,
    nesl_processor_execute_rotate,
    nesl_processor_execute_rotate,
    nesl_processor_execute_return,
    nesl_processor_execute_return,
    nesl_processor_execute_arithmetic,
    nesl_processor_execute_set,
    nesl_processor_execute_set,
    nesl_processor_execute_set,
    nesl_processor_execute_store,
    nesl_processor_execute_store,
    nesl_processor_execute_store,
    nesl_processor_execute_transfer,
    nesl_processor_execute_transfer,
    nesl_processor_execute_transfer,
    nesl_processor_execute_transfer,
    nesl_processor_execute_transfer,
    nesl_processor_execute_transfer,
    nesl_processor_execute_no_operation,
    };

static void nesl_processor_instruction(nesl_processor_t *processor)
{
    uint8_t opcode;
    nesl_operand_t operand = {};
    const nesl_instruction_t *instruction = &INSTRUCTION[(opcode = nesl_processor_fetch(processor))];

    ADDRESS[instruction->mode](processor, &operand);
    processor->cycle = instruction->cycles;
    EXECUTE[instruction->type](processor, instruction, &operand);
}

static void nesl_processor_interrupt_maskable(nesl_processor_t *processor)
{
    processor->event.maskable = false;
    nesl_processor_push_word(processor, processor->state.program_counter.word);
    nesl_processor_push(processor, processor->state.status.raw);
    processor->state.program_counter.word = nesl_processor_read_word(processor, 0xFFFE);
    processor->state.status.interrupt_disable = true;
    processor->cycle = 7;
}

static void nesl_processor_interrupt_non_maskable(nesl_processor_t *processor)
{
    processor->event.non_maskable = false;
    nesl_processor_push_word(processor, processor->state.program_counter.word);
    nesl_processor_push(processor, processor->state.status.raw);
    processor->state.program_counter.word = nesl_processor_read_word(processor, 0xFFFA);
    processor->state.status.interrupt_disable = true;
    processor->cycle = 7;
}

static void nesl_processor_transfer(nesl_processor_t *processor, uint64_t cycle)
{

    if(processor->event.transfer_sync) {

        if(cycle % 2) {
            processor->event.transfer_sync = false;
        }

        ++processor->cycle;
    } else {

        if(!(cycle % 2)) {
            processor->transfer.data = nesl_bus_read(NESL_BUS_PROCESSOR, processor->transfer.source.word++);
            ++processor->cycle;
        } else {
            nesl_bus_write(NESL_BUS_VIDEO_OAM, processor->transfer.destination.low++, processor->transfer.data);
            ++processor->cycle;

            if(processor->transfer.destination.low == 0x00) {
                processor->event.transfer = false;
                processor->transfer.destination.word = 0;
                processor->transfer.source.word = 0;
            }
        }
    }
}

void nesl_processor_cycle(nesl_processor_t *processor, uint64_t cycle)
{
    if(!(cycle % 3)) {

        if(!processor->cycle) {

            if(processor->event.transfer) {
                nesl_processor_transfer(processor, cycle);
            }

            if(!processor->event.transfer) {

                if(processor->event.non_maskable) {
                    nesl_processor_interrupt_non_maskable(processor);
                } else if(processor->event.maskable
                        && !processor->state.status.interrupt_disable) {
                    nesl_processor_interrupt_maskable(processor);
                } else {
                    nesl_processor_instruction(processor);
                }
            }
        }

        --processor->cycle;
    }
}

int nesl_processor_initialize(nesl_processor_t *processor)
{
    nesl_processor_reset(processor);

    return NESL_SUCCESS;
}

int nesl_processor_interrupt(nesl_processor_t *processor, bool maskable)
{

    if(maskable) {
        processor->event.maskable = true;
    } else {
        processor->event.non_maskable = true;
    }

    return NESL_SUCCESS;
}

uint8_t nesl_processor_read(nesl_processor_t *processor, uint16_t address)
{
    uint8_t result = 0;

    switch(address) {
        case 0x0000 ... 0x1FFF:
            result = processor->ram[address & 0x07FF];
            break;
        default:
            break;
    }

    return result;
}

int nesl_processor_reset(nesl_processor_t *processor)
{
    memset(processor, 0, sizeof(*processor));
    nesl_processor_push_word(processor, processor->state.program_counter.word);
    nesl_processor_push(processor, processor->state.status.raw);
    processor->state.program_counter.word = nesl_processor_read_word(processor, 0xFFFC);
    processor->state.status.interrupt_disable = true;
    processor->state.status.breakpoint = true;
    processor->state.status.unused = true;
    processor->cycle = 7;

    return NESL_SUCCESS;
}

void nesl_processor_uninitialize(nesl_processor_t *processor)
{
    memset(processor, 0, sizeof(*processor));
}

void nesl_processor_write(nesl_processor_t *processor, uint16_t address, uint8_t data)
{

    switch(address) {
        case 0x0000 ... 0x1FFF:
            processor->ram[address & 0x07FF] = data;
            break;
        case 0x4014:
            processor->event.transfer = true;
            processor->event.transfer_sync = true;
            processor->transfer.source.word = data << 8;
            processor->transfer.destination.word = 0;
            break;
        default:
            break;
    }
}

#ifdef __cplusplus
}
#endif /* __cplusplus */
