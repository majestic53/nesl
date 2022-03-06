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
 * @file NESL_processor.c
 * @brief Processor subsystem.
 */

#include "../../include/system/NESL_processor.h"

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
 * @struct nesl_operand_t
 * @brief Processor operand data.
 */
typedef struct {
    nesl_register_t data;       /*< Immediate data */
    nesl_register_t effective;  /*< Effective address to data */
    nesl_register_t indirect;   /*< Indirect address to data */
    bool page_cross;            /*< Page boundary crossed */
} nesl_operand_t;

/**
 * @brief Processor execution function.
 * @param processor Pointer to processor context
 * @param instruction Constant pointer to instruction data
 * @param operand Constant pointer to operand data
 */
typedef void (*NESL_ProcessorExecute)(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand);

/**
 * @brief Processor operand function.
 * @param processor Pointer to processor context
 * @param operand Pointer to operand data
 */
typedef void (*NESL_ProcessorOperand)(nesl_processor_t *processor, nesl_operand_t *operand);

/**
 * @brief Processor operation function.
 * @param processor Pointer to processor context
 * @param left Pointer to left register context
 * @param right Constant pointer to right register context
 */
typedef void (*NESL_ProcessorOperation)(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

static uint8_t NESL_ProcessorFetch(nesl_processor_t *processor)
{
    return NESL_BusRead(NESL_BUS_PROCESSOR, processor->state.program_counter.word++);
}

static uint16_t NESL_ProcessorFetchWord(nesl_processor_t *processor)
{
    return NESL_ProcessorFetch(processor) | (NESL_ProcessorFetch(processor) << 8);
}

static uint8_t NESL_ProcessorPull(nesl_processor_t *processor)
{
    return NESL_BusRead(NESL_BUS_PROCESSOR, 0x0100 | ++processor->state.stack_pointer.low);
}

static uint16_t NESL_ProcessorPullWord(nesl_processor_t *processor)
{
    return NESL_ProcessorPull(processor) | (NESL_ProcessorPull(processor) << 8);
}

static void NESL_ProcessorPush(nesl_processor_t *processor, uint8_t data)
{
    NESL_BusWrite(NESL_BUS_PROCESSOR, 0x0100 | processor->state.stack_pointer.low--, data);
}

static void NESL_ProcessorPushWord(nesl_processor_t *processor, uint16_t data)
{
    NESL_ProcessorPush(processor, data >> 8);
    NESL_ProcessorPush(processor, data);
}

static uint16_t NESL_ProcessorReadWord(nesl_processor_t *processor, uint16_t address)
{
    return NESL_BusRead(NESL_BUS_PROCESSOR, address) | (NESL_BusRead(NESL_BUS_PROCESSOR, address + 1) << 8);
}

static void NESL_ProcessorOperationAddCarry(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    nesl_register_t data = {};

    data.word = left->low + right->low + (processor->state.status.carry ? 1 : 0);
    processor->state.status.carry = data.high > 0;
    processor->state.status.overflow = !(left->bit_7 ^ right->bit_7) && (left->bit_7 ^ data.bit_7);
    left->low = data.low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void NESL_ProcessorOperationCompare(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    nesl_register_t data = {};

    data.low = right->low;
    processor->state.status.carry = (right->low >= left->low);
    data.low -= left->low;
    left->low = data.low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void NESL_ProcessorOperationDecrement(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    --left->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void NESL_ProcessorOperationIncrement(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    ++left->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void NESL_ProcessorOperationLogicalAnd(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    left->low &= right->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void NESL_ProcessorOperationLogicalExclusiveOr(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    left->low ^= right->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void NESL_ProcessorOperationLogicalOr(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    left->low |= right->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void NESL_ProcessorOperationRotateLeft(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    bool carry = processor->state.status.carry;

    processor->state.status.carry = left->bit_7;
    left->low <<= 1;
    left->bit_0 = carry;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void NESL_ProcessorOperationRotateRight(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    bool carry = processor->state.status.carry;

    processor->state.status.carry = left->bit_0;
    left->low >>= 1;
    left->bit_7 = carry;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void NESL_ProcessorOperationShiftLeft(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    processor->state.status.carry = left->bit_7;
    left->low <<= 1;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void NESL_ProcessorOperationShiftRight(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    processor->state.status.carry = left->bit_0;
    left->low >>= 1;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

static void NESL_ProcessorOperationTransfer(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{

    if(left != &processor->state.stack_pointer) {
        processor->state.status.negative = right->bit_7;
        processor->state.status.zero = !right->low;
    }

    left->low = right->low;
}

static void NESL_ProcessorExecuteArithmetic(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_OPERAND_IMMEDIATE) {
        data.low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->effective.word);
    }

    if(instruction->type == NESL_INSTRUCTION_SBC) {
        data.low = ~data.low;
    }

    NESL_ProcessorOperationAddCarry(processor, &processor->state.accumulator, &data);

    if(operand->page_cross) {
        ++processor->cycle;
    }
}

static void NESL_ProcessorExecuteBit(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->effective.word) };

    processor->state.status.negative = data.bit_7;
    processor->state.status.overflow = data.bit_6;
    processor->state.status.zero = !(processor->state.accumulator.low & data.low);
}

static void NESL_ProcessorExecuteBranch(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
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

static void NESL_ProcessorExecuteBreakpoint(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    NESL_ProcessorPushWord(processor, processor->state.program_counter.word + 1);
    processor->state.status.breakpoint = true;
    NESL_ProcessorPush(processor, processor->state.status.raw);
    processor->state.status.interrupt_disable = true;
    processor->state.program_counter.word = NESL_ProcessorReadWord(processor, 0xFFFE);
}

static void NESL_ProcessorExecuteClear(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
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

static void NESL_ProcessorExecuteCompare(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_OPERAND_IMMEDIATE) {
        data.low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case NESL_INSTRUCTION_CMP:
            NESL_ProcessorOperationCompare(processor, &data, &processor->state.accumulator);
            break;
        case NESL_INSTRUCTION_CPX:
            NESL_ProcessorOperationCompare(processor, &data, &processor->state.index.x);
            break;
        case NESL_INSTRUCTION_CPY:
            NESL_ProcessorOperationCompare(processor, &data, &processor->state.index.y);
            break;
        default:
            break;
    }

    if(operand->page_cross) {
        ++processor->cycle;
    }
}

static void NESL_ProcessorExecuteDecrement(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = {};

    switch(instruction->type) {
        case NESL_INSTRUCTION_DEC:
            data.low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->effective.word);
            NESL_ProcessorOperationDecrement(processor, &data, NULL);
            NESL_BusWrite(NESL_BUS_PROCESSOR, operand->effective.word, data.low);
            break;
        case NESL_INSTRUCTION_DEX:
            NESL_ProcessorOperationDecrement(processor, &processor->state.index.x, NULL);
            break;
        case NESL_INSTRUCTION_DEY:
            NESL_ProcessorOperationDecrement(processor, &processor->state.index.y, NULL);
            break;
        default:
            break;
    }
}

static void NESL_ProcessorExecuteIncrement(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = {};

    switch(instruction->type) {
        case NESL_INSTRUCTION_INC:
            data.low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->effective.word);
            NESL_ProcessorOperationIncrement(processor, &data, NULL);
            NESL_BusWrite(NESL_BUS_PROCESSOR, operand->effective.word, data.low);
            break;
        case NESL_INSTRUCTION_INX:
            NESL_ProcessorOperationIncrement(processor, &processor->state.index.x, NULL);
            break;
        case NESL_INSTRUCTION_INY:
            NESL_ProcessorOperationIncrement(processor, &processor->state.index.y, NULL);
            break;
        default:
            break;
    }
}

static void NESL_ProcessorExecuteJump(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    if(instruction->type == NESL_INSTRUCTION_JSR) {
        NESL_ProcessorPushWord(processor, processor->state.program_counter.word - 1);
    }

    processor->state.program_counter.word = operand->effective.word;
}

static void NESL_ProcessorExecuteLoad(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_OPERAND_IMMEDIATE) {
        data.low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->effective.word);
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

static void NESL_ProcessorExecuteLogical(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_OPERAND_IMMEDIATE) {
        data.low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case NESL_INSTRUCTION_AND:
            NESL_ProcessorOperationLogicalAnd(processor, &processor->state.accumulator, &data);
            break;
        case NESL_INSTRUCTION_EOR:
            NESL_ProcessorOperationLogicalExclusiveOr(processor, &processor->state.accumulator, &data);
            break;
        case NESL_INSTRUCTION_ORA:
            NESL_ProcessorOperationLogicalOr(processor, &processor->state.accumulator, &data);
            break;
        default:
            break;
    }

    if(operand->page_cross) {
        ++processor->cycle;
    }
}

static void NESL_ProcessorExecuteNoOperation(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    return;
}

static void NESL_ProcessorExecutePull(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case NESL_INSTRUCTION_PLA:
            processor->state.accumulator.low = NESL_ProcessorPull(processor);
            processor->state.status.negative = processor->state.accumulator.bit_7;
            processor->state.status.zero = !processor->state.accumulator.low;
            break;
        case NESL_INSTRUCTION_PLP:
            processor->state.status.raw = NESL_ProcessorPull(processor);
            processor->state.status.unused = true;
            break;
        default:
            break;
    }
}

static void NESL_ProcessorExecutePush(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case NESL_INSTRUCTION_PHA:
            NESL_ProcessorPush(processor, processor->state.accumulator.low);
            break;
        case NESL_INSTRUCTION_PHP:
            NESL_ProcessorPush(processor, processor->state.status.raw);
            break;
        default:
            break;
    }
}

static void NESL_ProcessorExecuteReturn(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case NESL_INSTRUCTION_RTI:
            processor->state.status.raw = NESL_ProcessorPull(processor);
            processor->state.status.unused = true;
            processor->state.program_counter.word = NESL_ProcessorPullWord(processor);
            break;
        case NESL_INSTRUCTION_RTS:
            processor->state.program_counter.word = NESL_ProcessorPullWord(processor) + 1;
            break;
        default:
            break;
    }
}

static void NESL_ProcessorExecuteRotate(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_OPERAND_ACCUMULATOR) {
        data.low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case NESL_INSTRUCTION_ROL:
            NESL_ProcessorOperationRotateLeft(processor, &data, NULL);
            break;
        case NESL_INSTRUCTION_ROR:
            NESL_ProcessorOperationRotateRight(processor, &data, NULL);
            break;
        default:
            break;
    }

    if(instruction->mode != NESL_OPERAND_ACCUMULATOR) {
        NESL_BusWrite(NESL_BUS_PROCESSOR, operand->effective.word, data.low);
    } else {
        processor->state.accumulator.low = data.low;
    }
}

static void NESL_ProcessorExecuteSet(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
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

static void NESL_ProcessorExecuteShift(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != NESL_OPERAND_ACCUMULATOR) {
        data.low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case NESL_INSTRUCTION_ASL:
            NESL_ProcessorOperationShiftLeft(processor, &data, NULL);
            break;
        case NESL_INSTRUCTION_LSR:
            NESL_ProcessorOperationShiftRight(processor, &data, NULL);
            break;
        default:
            break;
    }

    if(instruction->mode != NESL_OPERAND_ACCUMULATOR) {
        NESL_BusWrite(NESL_BUS_PROCESSOR, operand->effective.word, data.low);
    } else {
        processor->state.accumulator.low = data.low;
    }
}

static void NESL_ProcessorExecuteStore(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
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

    NESL_BusWrite(NESL_BUS_PROCESSOR, operand->effective.word, data.low);
}

static void NESL_ProcessorExecuteTransfer(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case NESL_INSTRUCTION_TAX:
            NESL_ProcessorOperationTransfer(processor, &processor->state.index.x, &processor->state.accumulator);
            break;
        case NESL_INSTRUCTION_TAY:
            NESL_ProcessorOperationTransfer(processor, &processor->state.index.y, &processor->state.accumulator);
            break;
        case NESL_INSTRUCTION_TSX:
            NESL_ProcessorOperationTransfer(processor, &processor->state.index.x, &processor->state.stack_pointer);
            break;
        case NESL_INSTRUCTION_TXA:
            NESL_ProcessorOperationTransfer(processor, &processor->state.accumulator, &processor->state.index.x);
            break;
        case NESL_INSTRUCTION_TXS:
            NESL_ProcessorOperationTransfer(processor, &processor->state.stack_pointer, &processor->state.index.x);
            break;
        case NESL_INSTRUCTION_TYA:
            NESL_ProcessorOperationTransfer(processor, &processor->state.accumulator, &processor->state.index.y);
            break;
        default:
            break;
    }
}

static void NESL_ProcessorOperandAbsolute(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = NESL_ProcessorFetchWord(processor);
    operand->effective.word = operand->data.word;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void NESL_ProcessorOperandAbsoluteX(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = NESL_ProcessorFetchWord(processor);
    operand->effective.word = operand->data.word + processor->state.index.x.low;
    operand->indirect.word = 0;
    operand->page_cross = (operand->effective.high != operand->data.high);
}

static void NESL_ProcessorOperandAbsoluteY(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = NESL_ProcessorFetchWord(processor);
    operand->effective.word = operand->data.word + processor->state.index.y.low;
    operand->indirect.word = 0;
    operand->page_cross = (operand->effective.high != operand->data.high);
}

static void NESL_ProcessorOperandAccumulator(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = processor->state.accumulator.low;
    operand->effective.word = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void NESL_ProcessorOperandImmediate(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = NESL_ProcessorFetch(processor);
    operand->effective.word = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void NESL_ProcessorOperandImplied(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = 0;
    operand->effective.word = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void NESL_ProcessorOperandIndirect(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = NESL_ProcessorFetchWord(processor);
    operand->indirect.word = operand->data.word;

    if(operand->indirect.low == 0xFF) {
        operand->effective.low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->indirect.word);
        operand->effective.high = NESL_BusRead(NESL_BUS_PROCESSOR, operand->indirect.word & 0xFF00);
    } else {
        operand->effective.word = NESL_ProcessorReadWord(processor, operand->indirect.word);
    }

    operand->page_cross = false;
}

static void NESL_ProcessorOperandIndirectX(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = NESL_ProcessorFetch(processor);
    operand->indirect.low = (operand->data.word + processor->state.index.x.low);
    operand->indirect.high = 0;
    operand->effective.low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->indirect.low);
    operand->effective.high = NESL_BusRead(NESL_BUS_PROCESSOR, (operand->indirect.low + 1) & 0xFF);
    operand->page_cross = false;
}

static void NESL_ProcessorOperandIndirectY(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = NESL_ProcessorFetch(processor);
    operand->indirect.low = NESL_BusRead(NESL_BUS_PROCESSOR, operand->data.low);
    operand->indirect.high = NESL_BusRead(NESL_BUS_PROCESSOR, (operand->data.low + 1) & 0xFF);
    operand->effective.word = operand->indirect.word + processor->state.index.y.low;
    operand->page_cross = (operand->effective.high != operand->indirect.high);
}

static void NESL_ProcessorOperandRelative(nesl_processor_t *processor, nesl_operand_t *operand)

{
    operand->data.word = NESL_ProcessorFetch(processor);

    if(operand->data.bit_7) {
        operand->data.high = 0xFF;
    }

    operand->effective.word = processor->state.program_counter.word + operand->data.word;
    operand->indirect.word = 0;
    operand->page_cross = (operand->effective.high != processor->state.program_counter.high);
}

static void NESL_ProcessorOperandZeropage(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = NESL_ProcessorFetch(processor);
    operand->effective.word = operand->data.word;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void NESL_ProcessorOperandZeropageX(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = NESL_ProcessorFetch(processor);
    operand->effective.low = operand->data.low + processor->state.index.x.low;
    operand->effective.high = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void NESL_ProcessorOperandZeropageY(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = NESL_ProcessorFetch(processor);
    operand->effective.low = operand->data.low + processor->state.index.y.low;
    operand->effective.high = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

static void NESL_ProcessorInstruction(nesl_processor_t *processor)
{
    static const NESL_ProcessorExecute EXECUTE[] = {
        NESL_ProcessorExecuteArithmetic, NESL_ProcessorExecuteLogical, NESL_ProcessorExecuteShift, NESL_ProcessorExecuteBranch,
        NESL_ProcessorExecuteBranch, NESL_ProcessorExecuteBranch, NESL_ProcessorExecuteBit, NESL_ProcessorExecuteBranch,
        NESL_ProcessorExecuteBranch, NESL_ProcessorExecuteBranch, NESL_ProcessorExecuteBreakpoint, NESL_ProcessorExecuteBranch,
        NESL_ProcessorExecuteBranch, NESL_ProcessorExecuteClear, NESL_ProcessorExecuteClear, NESL_ProcessorExecuteClear,
        NESL_ProcessorExecuteClear, NESL_ProcessorExecuteCompare, NESL_ProcessorExecuteCompare, NESL_ProcessorExecuteCompare,
        NESL_ProcessorExecuteDecrement, NESL_ProcessorExecuteDecrement, NESL_ProcessorExecuteDecrement, NESL_ProcessorExecuteLogical,
        NESL_ProcessorExecuteIncrement, NESL_ProcessorExecuteIncrement, NESL_ProcessorExecuteIncrement, NESL_ProcessorExecuteJump,
        NESL_ProcessorExecuteJump, NESL_ProcessorExecuteLoad, NESL_ProcessorExecuteLoad, NESL_ProcessorExecuteLoad,
        NESL_ProcessorExecuteShift, NESL_ProcessorExecuteNoOperation, NESL_ProcessorExecuteLogical, NESL_ProcessorExecutePush,
        NESL_ProcessorExecutePush, NESL_ProcessorExecutePull, NESL_ProcessorExecutePull, NESL_ProcessorExecuteRotate,
        NESL_ProcessorExecuteRotate, NESL_ProcessorExecuteReturn, NESL_ProcessorExecuteReturn, NESL_ProcessorExecuteArithmetic,
        NESL_ProcessorExecuteSet, NESL_ProcessorExecuteSet, NESL_ProcessorExecuteSet, NESL_ProcessorExecuteStore,
        NESL_ProcessorExecuteStore, NESL_ProcessorExecuteStore, NESL_ProcessorExecuteTransfer, NESL_ProcessorExecuteTransfer,
        NESL_ProcessorExecuteTransfer, NESL_ProcessorExecuteTransfer, NESL_ProcessorExecuteTransfer, NESL_ProcessorExecuteTransfer,
        NESL_ProcessorExecuteNoOperation,
        };

    static const nesl_instruction_t INSTRUCTION[] = {
        { NESL_INSTRUCTION_BRK, NESL_OPERAND_IMPLIED, 7 }, { NESL_INSTRUCTION_ORA, NESL_OPERAND_INDIRECT_X, 6 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_X, 8 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE, 3 }, { NESL_INSTRUCTION_ORA, NESL_OPERAND_ZEROPAGE, 3 },
        { NESL_INSTRUCTION_ASL, NESL_OPERAND_ZEROPAGE, 5 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE, 5 },
        { NESL_INSTRUCTION_PHP, NESL_OPERAND_IMPLIED, 3 }, { NESL_INSTRUCTION_ORA, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_ASL, NESL_OPERAND_ACCUMULATOR, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE, 4 }, { NESL_INSTRUCTION_ORA, NESL_OPERAND_ABSOLUTE, 4 },
        { NESL_INSTRUCTION_ASL, NESL_OPERAND_ABSOLUTE, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE, 6 },
        { NESL_INSTRUCTION_BPL, NESL_OPERAND_RELATIVE, 2 }, { NESL_INSTRUCTION_ORA, NESL_OPERAND_INDIRECT_Y, 5 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_Y, 8 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 4 }, { NESL_INSTRUCTION_ORA, NESL_OPERAND_ZEROPAGE_X, 4 },
        { NESL_INSTRUCTION_ASL, NESL_OPERAND_ZEROPAGE_X, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 6 },
        { NESL_INSTRUCTION_CLC, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_ORA, NESL_OPERAND_ABSOLUTE_Y, 4 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_Y, 7 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 4 }, { NESL_INSTRUCTION_ORA, NESL_OPERAND_ABSOLUTE_X, 4 },
        { NESL_INSTRUCTION_ASL, NESL_OPERAND_ABSOLUTE_X, 7 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 7 },
        { NESL_INSTRUCTION_JSR, NESL_OPERAND_ABSOLUTE, 6 }, { NESL_INSTRUCTION_AND, NESL_OPERAND_INDIRECT_X, 6 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_X, 8 },
        { NESL_INSTRUCTION_BIT, NESL_OPERAND_ZEROPAGE, 3 }, { NESL_INSTRUCTION_AND, NESL_OPERAND_ZEROPAGE, 3 },
        { NESL_INSTRUCTION_ROL, NESL_OPERAND_ZEROPAGE, 5 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE, 5 },
        { NESL_INSTRUCTION_PLP, NESL_OPERAND_IMPLIED, 4 }, { NESL_INSTRUCTION_AND, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_ROL, NESL_OPERAND_ACCUMULATOR, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_BIT, NESL_OPERAND_ABSOLUTE, 4 }, { NESL_INSTRUCTION_AND, NESL_OPERAND_ABSOLUTE, 4 },
        { NESL_INSTRUCTION_ROL, NESL_OPERAND_ABSOLUTE, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE, 6 },
        { NESL_INSTRUCTION_BMI, NESL_OPERAND_RELATIVE, 2 }, { NESL_INSTRUCTION_AND, NESL_OPERAND_INDIRECT_Y, 5 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_Y, 8 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 4 }, { NESL_INSTRUCTION_AND, NESL_OPERAND_ZEROPAGE_X, 4 },
        { NESL_INSTRUCTION_ROL, NESL_OPERAND_ZEROPAGE_X, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 6 },
        { NESL_INSTRUCTION_SEC, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_AND, NESL_OPERAND_ABSOLUTE_Y, 4 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_Y, 7 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 4 }, { NESL_INSTRUCTION_AND, NESL_OPERAND_ABSOLUTE_X, 4 },
        { NESL_INSTRUCTION_ROL, NESL_OPERAND_ABSOLUTE_X, 7 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 7 },
        { NESL_INSTRUCTION_RTI, NESL_OPERAND_IMPLIED, 6 }, { NESL_INSTRUCTION_EOR, NESL_OPERAND_INDIRECT_X, 6 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_X, 8 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE, 3 }, { NESL_INSTRUCTION_EOR, NESL_OPERAND_ZEROPAGE, 3 },
        { NESL_INSTRUCTION_LSR, NESL_OPERAND_ZEROPAGE, 5 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE, 5 },
        { NESL_INSTRUCTION_PHA, NESL_OPERAND_IMPLIED, 3 }, { NESL_INSTRUCTION_EOR, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_LSR, NESL_OPERAND_ACCUMULATOR, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_JMP, NESL_OPERAND_ABSOLUTE, 3 }, { NESL_INSTRUCTION_EOR, NESL_OPERAND_ABSOLUTE, 4 },
        { NESL_INSTRUCTION_LSR, NESL_OPERAND_ABSOLUTE, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE, 6 },
        { NESL_INSTRUCTION_BVC, NESL_OPERAND_RELATIVE, 2 }, { NESL_INSTRUCTION_EOR, NESL_OPERAND_INDIRECT_Y, 5 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_Y, 8 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 4 }, { NESL_INSTRUCTION_EOR, NESL_OPERAND_ZEROPAGE_X, 4 },
        { NESL_INSTRUCTION_LSR, NESL_OPERAND_ZEROPAGE_X, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 6 },
        { NESL_INSTRUCTION_CLI, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_EOR, NESL_OPERAND_ABSOLUTE_Y, 4 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_Y, 7 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 4 }, { NESL_INSTRUCTION_EOR, NESL_OPERAND_ABSOLUTE_X, 4 },
        { NESL_INSTRUCTION_LSR, NESL_OPERAND_ABSOLUTE_X, 7 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 7 },
        { NESL_INSTRUCTION_RTS, NESL_OPERAND_IMPLIED, 6 }, { NESL_INSTRUCTION_ADC, NESL_OPERAND_INDIRECT_X, 6 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_X, 8 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE, 3 }, { NESL_INSTRUCTION_ADC, NESL_OPERAND_ZEROPAGE, 3 },
        { NESL_INSTRUCTION_ROR, NESL_OPERAND_ZEROPAGE, 5 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE, 5 },
        { NESL_INSTRUCTION_PLA, NESL_OPERAND_IMPLIED, 4 }, { NESL_INSTRUCTION_ADC, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_ROR, NESL_OPERAND_ACCUMULATOR, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_JMP, NESL_OPERAND_INDIRECT, 5 }, { NESL_INSTRUCTION_ADC, NESL_OPERAND_ABSOLUTE, 4 },
        { NESL_INSTRUCTION_ROR, NESL_OPERAND_ABSOLUTE, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE, 6 },
        { NESL_INSTRUCTION_BVS, NESL_OPERAND_RELATIVE, 2 }, { NESL_INSTRUCTION_ADC, NESL_OPERAND_INDIRECT_Y, 5 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_Y, 8 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 4 }, { NESL_INSTRUCTION_ADC, NESL_OPERAND_ZEROPAGE_X, 4 },
        { NESL_INSTRUCTION_ROR, NESL_OPERAND_ZEROPAGE_X, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 6 },
        { NESL_INSTRUCTION_SEI, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_ADC, NESL_OPERAND_ABSOLUTE_Y, 4 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_Y, 7 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 4 }, { NESL_INSTRUCTION_ADC, NESL_OPERAND_ABSOLUTE_X, 4 },
        { NESL_INSTRUCTION_ROR, NESL_OPERAND_ABSOLUTE_X, 7 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 7 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_STA, NESL_OPERAND_INDIRECT_X, 6 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_X, 6 },
        { NESL_INSTRUCTION_STY, NESL_OPERAND_ZEROPAGE, 3 }, { NESL_INSTRUCTION_STA, NESL_OPERAND_ZEROPAGE, 3 },
        { NESL_INSTRUCTION_STX, NESL_OPERAND_ZEROPAGE, 3 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE, 3 },
        { NESL_INSTRUCTION_DEY, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_TXA, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_STY, NESL_OPERAND_ABSOLUTE, 4 }, { NESL_INSTRUCTION_STA, NESL_OPERAND_ABSOLUTE, 4 },
        { NESL_INSTRUCTION_STX, NESL_OPERAND_ABSOLUTE, 4 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE, 4 },
        { NESL_INSTRUCTION_BCC, NESL_OPERAND_RELATIVE, 2 }, { NESL_INSTRUCTION_STA, NESL_OPERAND_INDIRECT_Y, 6 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_Y, 6 },
        { NESL_INSTRUCTION_STY, NESL_OPERAND_ZEROPAGE_X, 4 }, { NESL_INSTRUCTION_STA, NESL_OPERAND_ZEROPAGE_X, 4 },
        { NESL_INSTRUCTION_STX, NESL_OPERAND_ZEROPAGE_Y, 4 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_Y, 4 },
        { NESL_INSTRUCTION_TYA, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_STA, NESL_OPERAND_ABSOLUTE_Y, 5 },
        { NESL_INSTRUCTION_TXS, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_Y, 5 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 5 }, { NESL_INSTRUCTION_STA, NESL_OPERAND_ABSOLUTE_X, 5 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_Y, 5 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_Y, 5 },
        { NESL_INSTRUCTION_LDY, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_LDA, NESL_OPERAND_INDIRECT_X, 6 },
        { NESL_INSTRUCTION_LDX, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_X, 6 },
        { NESL_INSTRUCTION_LDY, NESL_OPERAND_ZEROPAGE, 3 }, { NESL_INSTRUCTION_LDA, NESL_OPERAND_ZEROPAGE, 3 },
        { NESL_INSTRUCTION_LDX, NESL_OPERAND_ZEROPAGE, 3 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE, 3 },
        { NESL_INSTRUCTION_TAY, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_LDA, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_TAX, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_LDY, NESL_OPERAND_ABSOLUTE, 4 }, { NESL_INSTRUCTION_LDA, NESL_OPERAND_ABSOLUTE, 4 },
        { NESL_INSTRUCTION_LDX, NESL_OPERAND_ABSOLUTE, 4 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE, 4 },
        { NESL_INSTRUCTION_BCS, NESL_OPERAND_RELATIVE, 2 }, { NESL_INSTRUCTION_LDA, NESL_OPERAND_INDIRECT_Y, 5 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_Y, 5 },
        { NESL_INSTRUCTION_LDY, NESL_OPERAND_ZEROPAGE_X, 4 }, { NESL_INSTRUCTION_LDA, NESL_OPERAND_ZEROPAGE_X, 4 },
        { NESL_INSTRUCTION_LDX, NESL_OPERAND_ZEROPAGE_Y, 4 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_Y, 4 },
        { NESL_INSTRUCTION_CLV, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_LDA, NESL_OPERAND_ABSOLUTE_Y, 4 },
        { NESL_INSTRUCTION_TSX, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_Y, 4 },
        { NESL_INSTRUCTION_LDY, NESL_OPERAND_ABSOLUTE_X, 4 }, { NESL_INSTRUCTION_LDA, NESL_OPERAND_ABSOLUTE_X, 4 },
        { NESL_INSTRUCTION_LDX, NESL_OPERAND_ABSOLUTE_Y, 4 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_Y, 4 },
        { NESL_INSTRUCTION_CPY, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_CMP, NESL_OPERAND_INDIRECT_X, 6 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_X, 8 },
        { NESL_INSTRUCTION_CPY, NESL_OPERAND_ZEROPAGE, 3 }, { NESL_INSTRUCTION_CMP, NESL_OPERAND_ZEROPAGE, 3 },
        { NESL_INSTRUCTION_DEC, NESL_OPERAND_ZEROPAGE, 5 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE, 5 },
        { NESL_INSTRUCTION_INY, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_CMP, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_DEX, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_CPY, NESL_OPERAND_ABSOLUTE, 4 }, { NESL_INSTRUCTION_CMP, NESL_OPERAND_ABSOLUTE, 4 },
        { NESL_INSTRUCTION_DEC, NESL_OPERAND_ABSOLUTE, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE, 6 },
        { NESL_INSTRUCTION_BNE, NESL_OPERAND_RELATIVE, 2 }, { NESL_INSTRUCTION_CMP, NESL_OPERAND_INDIRECT_Y, 5 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_Y, 8 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 4 }, { NESL_INSTRUCTION_CMP, NESL_OPERAND_ZEROPAGE_X, 4 },
        { NESL_INSTRUCTION_DEC, NESL_OPERAND_ZEROPAGE_X, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 6 },
        { NESL_INSTRUCTION_CLD, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_CMP, NESL_OPERAND_ABSOLUTE_Y, 4 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_Y, 7 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 4 }, { NESL_INSTRUCTION_CMP, NESL_OPERAND_ABSOLUTE_X, 4 },
        { NESL_INSTRUCTION_DEC, NESL_OPERAND_ABSOLUTE_X, 7 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 7 },
        { NESL_INSTRUCTION_CPX, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_SBC, NESL_OPERAND_INDIRECT_X, 6 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_X, 8 },
        { NESL_INSTRUCTION_CPX, NESL_OPERAND_ZEROPAGE, 3 }, { NESL_INSTRUCTION_SBC, NESL_OPERAND_ZEROPAGE, 3 },
        { NESL_INSTRUCTION_INC, NESL_OPERAND_ZEROPAGE, 5 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE, 5 },
        { NESL_INSTRUCTION_INX, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_SBC, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 },
        { NESL_INSTRUCTION_CPX, NESL_OPERAND_ABSOLUTE, 4 }, { NESL_INSTRUCTION_SBC, NESL_OPERAND_ABSOLUTE, 4 },
        { NESL_INSTRUCTION_INC, NESL_OPERAND_ABSOLUTE, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE, 6 },
        { NESL_INSTRUCTION_BEQ, NESL_OPERAND_RELATIVE, 2 }, { NESL_INSTRUCTION_SBC, NESL_OPERAND_INDIRECT_Y, 5 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_INDIRECT_Y, 8 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 4 }, { NESL_INSTRUCTION_SBC, NESL_OPERAND_ZEROPAGE_X, 4 },
        { NESL_INSTRUCTION_INC, NESL_OPERAND_ZEROPAGE_X, 6 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ZEROPAGE_X, 6 },
        { NESL_INSTRUCTION_SED, NESL_OPERAND_IMPLIED, 2 }, { NESL_INSTRUCTION_SBC, NESL_OPERAND_ABSOLUTE_Y, 4 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_IMMEDIATE, 2 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_Y, 7 },
        { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 4 }, { NESL_INSTRUCTION_SBC, NESL_OPERAND_ABSOLUTE_X, 4 },
        { NESL_INSTRUCTION_INC, NESL_OPERAND_ABSOLUTE_X, 7 }, { NESL_INSTRUCTION_NOP, NESL_OPERAND_ABSOLUTE_X, 7 },
        };

    static const NESL_ProcessorOperand OPERAND[] = {
        NESL_ProcessorOperandAbsolute, NESL_ProcessorOperandAbsoluteX, NESL_ProcessorOperandAbsoluteY, NESL_ProcessorOperandAccumulator,
        NESL_ProcessorOperandImmediate, NESL_ProcessorOperandImplied, NESL_ProcessorOperandIndirect, NESL_ProcessorOperandIndirectX,
        NESL_ProcessorOperandIndirectY, NESL_ProcessorOperandRelative, NESL_ProcessorOperandZeropage, NESL_ProcessorOperandZeropageX,
        NESL_ProcessorOperandZeropageY,
        };

    uint8_t opcode;
    nesl_operand_t operand = {};
    const nesl_instruction_t *instruction = &INSTRUCTION[(opcode = NESL_ProcessorFetch(processor))];

    processor->cycle = instruction->cycles;
    OPERAND[instruction->mode](processor, &operand);
    EXECUTE[instruction->type](processor, instruction, &operand);
}

static void NESL_ProcessorInterruptMaskable(nesl_processor_t *processor)
{
    processor->interrupt.maskable = false;
    NESL_ProcessorPushWord(processor, processor->state.program_counter.word);
    NESL_ProcessorPush(processor, processor->state.status.raw);
    processor->state.program_counter.word = NESL_ProcessorReadWord(processor, 0xFFFE);
    processor->state.status.interrupt_disable = true;
    processor->cycle = 7;
}

static void NESL_ProcessorInterruptNonMaskable(nesl_processor_t *processor)
{
    processor->interrupt.non_maskable = false;
    NESL_ProcessorPushWord(processor, processor->state.program_counter.word);
    NESL_ProcessorPush(processor, processor->state.status.raw);
    processor->state.program_counter.word = NESL_ProcessorReadWord(processor, 0xFFFA);
    processor->state.status.interrupt_disable = true;
    processor->cycle = 7;
}

static void NESL_ProcessorTransfer(nesl_processor_t *processor, uint64_t cycle)
{

    if(processor->interrupt.transfer_sync) {

        if(cycle % 2) {
            processor->interrupt.transfer_sync = false;
        }

        ++processor->cycle;
    } else {

        if(!(cycle % 2)) {
            processor->transfer.data = NESL_BusRead(NESL_BUS_PROCESSOR, processor->transfer.source.word++);
            ++processor->cycle;
        } else {
            NESL_BusWrite(NESL_BUS_VIDEO_OAM, processor->transfer.destination.low++, processor->transfer.data);
            ++processor->cycle;

            if(processor->transfer.destination.low == 0x00) {
                processor->interrupt.transfer = false;
                processor->transfer.destination.word = 0;
                processor->transfer.source.word = 0;
            }
        }
    }
}

void NESL_ProcessorCycle(nesl_processor_t *processor, uint64_t cycle)
{

    if(!(cycle % 3)) {

        if(!processor->cycle) {

            if(processor->interrupt.transfer) {
                NESL_ProcessorTransfer(processor, cycle);
            }

            if(!processor->interrupt.transfer) {

                if(processor->interrupt.non_maskable) {
                    NESL_ProcessorInterruptNonMaskable(processor);
                } else if(processor->interrupt.maskable
                        && !processor->state.status.interrupt_disable) {
                    NESL_ProcessorInterruptMaskable(processor);
                } else {
                    NESL_ProcessorInstruction(processor);
                }
            }
        }

        --processor->cycle;
    }
}

nesl_error_e NESL_ProcessorInit(nesl_processor_t *processor)
{
    return NESL_ProcessorReset(processor);
}

nesl_error_e NESL_ProcessorInterrupt(nesl_processor_t *processor, bool maskable)
{

    if(maskable) {
        processor->interrupt.maskable = true;
    } else {
        processor->interrupt.non_maskable = true;
    }

    return NESL_SUCCESS;
}

uint8_t NESL_ProcessorRead(nesl_processor_t *processor, uint16_t address)
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

int NESL_ProcessorReset(nesl_processor_t *processor)
{
    memset(processor, 0, sizeof(*processor));
    NESL_ProcessorPushWord(processor, processor->state.program_counter.word);
    NESL_ProcessorPush(processor, processor->state.status.raw);
    processor->state.program_counter.word = NESL_ProcessorReadWord(processor, 0xFFFC);
    processor->state.status.interrupt_disable = true;
    processor->state.status.breakpoint = true;
    processor->state.status.unused = true;
    processor->cycle = 7;

    return NESL_SUCCESS;
}

void NESL_ProcessorUninit(nesl_processor_t *processor)
{
    memset(processor, 0, sizeof(*processor));
}

void NESL_ProcessorWrite(nesl_processor_t *processor, uint16_t address, uint8_t data)
{

    switch(address) {
        case 0x0000 ... 0x1FFF:
            processor->ram[address & 0x07FF] = data;
            break;
        case 0x4014:
            processor->interrupt.transfer = true;
            processor->interrupt.transfer_sync = true;
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
