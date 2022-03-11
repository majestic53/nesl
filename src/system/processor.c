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
 * @file processor.c
 * @brief Processor subsystem.
 */

#include <processor.h>

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
typedef void (*nesl_processor_execute)(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand);

/**
 * @brief Processor operand function.
 * @param processor Pointer to processor context
 * @param operand Pointer to operand data
 */
typedef void (*nesl_processor_operand)(nesl_processor_t *processor, nesl_operand_t *operand);

/**
 * @brief Processor operation function.
 * @param processor Pointer to processor context
 * @param left Pointer to left register context
 * @param right Constant pointer to right register context
 */
typedef void (*nesl_processor_operation)(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right);

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Fetch byte at processor program counter and post-increment.
 * @param processor Pointer to processor subsystem context
 * @return Byte at processor program counter
 */
static uint8_t nesl_processor_fetch(nesl_processor_t *processor)
{
    return nesl_bus_read(BUS_PROCESSOR, processor->state.program_counter.word++);
}

/**
 * @brief Fetch word at processor program counter and post-increment.
 * @param processor Pointer to processor subsystem context
 * @return Word at processor program counter
 */
static uint16_t nesl_processor_fetch_word(nesl_processor_t *processor)
{
    return nesl_processor_fetch(processor) | (nesl_processor_fetch(processor) << 8);
}

/**
 * @brief Pull byte from processor stack and pre-increment.
 * @param processor Pointer to processor subsystem context
 * @return Byte from processor stack
 */
static uint8_t nesl_processor_pull(nesl_processor_t *processor)
{
    return nesl_bus_read(BUS_PROCESSOR, 0x0100 | ++processor->state.stack_pointer.low);
}

/**
 * @brief Pull word from processor stack and pre-increment.
 * @param processor Pointer to processor subsystem context
 * @return Word from processor stack
 */
static uint16_t nesl_processor_pull_word(nesl_processor_t *processor)
{
    return nesl_processor_pull(processor) | (nesl_processor_pull(processor) << 8);
}

/**
 * @brief Push byte to processor stack and post-decrement.
 * @param processor Pointer to processor subsystem context
 * @param data Byte to processor stack
 */
static void nesl_processor_push(nesl_processor_t *processor, uint8_t data)
{
    nesl_bus_write(BUS_PROCESSOR, 0x0100 | processor->state.stack_pointer.low--, data);
}

/**
 * @brief Push word to processor stack and post-decrement.
 * @param processor Pointer to processor subsystem context
 * @param data Word to processor stack
 */
static void nesl_processor_push_word(nesl_processor_t *processor, uint16_t data)
{
    nesl_processor_push(processor, data >> 8);
    nesl_processor_push(processor, data);
}

/**
 * @brief Read word from processor bus.
 * @param processor Pointer to processor subsystem context
 * @param address Processor bus address
 * @return Word at processor program counter
 */
static uint16_t nesl_processor_read_word(nesl_processor_t *processor, uint16_t address)
{
    return nesl_bus_read(BUS_PROCESSOR, address) | (nesl_bus_read(BUS_PROCESSOR, address + 1) << 8);
}

/**
 * @brief Perform add-carry operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
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

/**
 * @brief Perform compare operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
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

/**
 * @brief Perform decrement operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
static void nesl_processor_operation_decrement(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    --left->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

/**
 * @brief Perform increment operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
static void nesl_processor_operation_increment(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    ++left->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

/**
 * @brief Perform logical and operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
static void nesl_processor_operation_logical_and(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    left->low &= right->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

/**
 * @brief Perform logical exclusive-or operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
static void nesl_processor_operation_logical_exclusive_or(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    left->low ^= right->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

/**
 * @brief Perform logical or operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
static void nesl_processor_operation_logical_or(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    left->low |= right->low;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

/**
 * @brief Perform rotate-left operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
static void nesl_processor_operation_rotate_left(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    bool carry = processor->state.status.carry;

    processor->state.status.carry = left->bit_7;
    left->low <<= 1;
    left->bit_0 = carry;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

/**
 * @brief Perform rotate-right operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
static void nesl_processor_operation_rotate_right(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    bool carry = processor->state.status.carry;

    processor->state.status.carry = left->bit_0;
    left->low >>= 1;
    left->bit_7 = carry;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

/**
 * @brief Perform shift-left operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
static void nesl_processor_operation_shift_left(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    processor->state.status.carry = left->bit_7;
    left->low <<= 1;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

/**
 * @brief Perform shift-right operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
static void nesl_processor_operation_shift_right(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{
    processor->state.status.carry = left->bit_0;
    left->low >>= 1;
    processor->state.status.negative = left->bit_7;
    processor->state.status.zero = !left->low;
}

/**
 * @brief Perform transfer operation.
 * @param processor Pointer to processor subsystem context
 * @param left Left operand
 * @param right Right operand
 */
static void nesl_processor_operation_transfer(nesl_processor_t *processor, nesl_register_t *left, const nesl_register_t *right)
{

    if(left != &processor->state.stack_pointer) {
        processor->state.status.negative = right->bit_7;
        processor->state.status.zero = !right->low;
    }

    left->low = right->low;
}

/**
 * @brief Execute arithetic instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_arithmetic(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != OPERAND_IMMEDIATE) {
        data.low = nesl_bus_read(BUS_PROCESSOR, operand->effective.word);
    }

    if(instruction->type == INSTRUCTION_SBC) {
        data.low = ~data.low;
    }

    nesl_processor_operation_add_carry(processor, &processor->state.accumulator, &data);

    if(operand->page_cross) {
        ++processor->cycle;
    }
}

/**
 * @brief Execute bit instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_bit(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = nesl_bus_read(BUS_PROCESSOR, operand->effective.word) };

    processor->state.status.negative = data.bit_7;
    processor->state.status.overflow = data.bit_6;
    processor->state.status.zero = !(processor->state.accumulator.low & data.low);
}

/**
 * @brief Execute branch instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_branch(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    bool branch = false;

    switch(instruction->type) {
        case INSTRUCTION_BCC:
            branch = !processor->state.status.carry;
            break;
        case INSTRUCTION_BCS:
            branch = processor->state.status.carry;
            break;
        case INSTRUCTION_BEQ:
            branch = processor->state.status.zero;
            break;
        case INSTRUCTION_BMI:
            branch = processor->state.status.negative;
            break;
        case INSTRUCTION_BNE:
            branch = !processor->state.status.zero;
            break;
        case INSTRUCTION_BPL:
            branch = !processor->state.status.negative;
            break;
        case INSTRUCTION_BVC:
            branch = !processor->state.status.overflow;
            break;
        case INSTRUCTION_BVS:
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

/**
 * @brief Execute break instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_breakpoint(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_processor_push_word(processor, processor->state.program_counter.word + 1);
    processor->state.status.breakpoint = true;
    nesl_processor_push(processor, processor->state.status.raw);
    processor->state.status.interrupt_disable = true;
    processor->state.program_counter.word = nesl_processor_read_word(processor, 0xFFFE);
}

/**
 * @brief Execute clear bit instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_clear(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case INSTRUCTION_CLC:
            processor->state.status.carry = false;
            break;
        case INSTRUCTION_CLD:
            processor->state.status.decimal = false;
            break;
        case INSTRUCTION_CLI:
            processor->state.status.interrupt_disable = false;
            break;
        case INSTRUCTION_CLV:
            processor->state.status.overflow = false;
            break;
        default:
            break;
    }
}

/**
 * @brief Execute compare instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_compare(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != OPERAND_IMMEDIATE) {
        data.low = nesl_bus_read(BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case INSTRUCTION_CMP:
            nesl_processor_operation_compare(processor, &data, &processor->state.accumulator);
            break;
        case INSTRUCTION_CPX:
            nesl_processor_operation_compare(processor, &data, &processor->state.index.x);
            break;
        case INSTRUCTION_CPY:
            nesl_processor_operation_compare(processor, &data, &processor->state.index.y);
            break;
        default:
            break;
    }

    if(operand->page_cross) {
        ++processor->cycle;
    }
}

/**
 * @brief Execute decrement instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_decrement(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = {};

    switch(instruction->type) {
        case INSTRUCTION_DEC:
            data.low = nesl_bus_read(BUS_PROCESSOR, operand->effective.word);
            nesl_processor_operation_decrement(processor, &data, NULL);
            nesl_bus_write(BUS_PROCESSOR, operand->effective.word, data.low);
            break;
        case INSTRUCTION_DEX:
            nesl_processor_operation_decrement(processor, &processor->state.index.x, NULL);
            break;
        case INSTRUCTION_DEY:
            nesl_processor_operation_decrement(processor, &processor->state.index.y, NULL);
            break;
        default:
            break;
    }
}

/**
 * @brief Execute increment instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_increment(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = {};

    switch(instruction->type) {
        case INSTRUCTION_INC:
            data.low = nesl_bus_read(BUS_PROCESSOR, operand->effective.word);
            nesl_processor_operation_increment(processor, &data, NULL);
            nesl_bus_write(BUS_PROCESSOR, operand->effective.word, data.low);
            break;
        case INSTRUCTION_INX:
            nesl_processor_operation_increment(processor, &processor->state.index.x, NULL);
            break;
        case INSTRUCTION_INY:
            nesl_processor_operation_increment(processor, &processor->state.index.y, NULL);
            break;
        default:
            break;
    }
}

/**
 * @brief Execute jump instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_jump(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    if(instruction->type == INSTRUCTION_JSR) {
        nesl_processor_push_word(processor, processor->state.program_counter.word - 1);
    }

    processor->state.program_counter.word = operand->effective.word;
}

/**
 * @brief Execute load instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_load(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != OPERAND_IMMEDIATE) {
        data.low = nesl_bus_read(BUS_PROCESSOR, operand->effective.word);
    }

    processor->state.status.negative = data.bit_7;
    processor->state.status.zero = !data.low;

    switch(instruction->type) {
        case INSTRUCTION_LDA:
            processor->state.accumulator.low = data.low;
            break;
        case INSTRUCTION_LDX:
            processor->state.index.x.low = data.low;
            break;
        case INSTRUCTION_LDY:
            processor->state.index.y.low = data.low;
            break;
        default:
            break;
    }

    if(operand->page_cross) {
        ++processor->cycle;
    }
}

/**
 * @brief Execute logical instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_logical(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != OPERAND_IMMEDIATE) {
        data.low = nesl_bus_read(BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case INSTRUCTION_AND:
            nesl_processor_operation_logical_and(processor, &processor->state.accumulator, &data);
            break;
        case INSTRUCTION_EOR:
            nesl_processor_operation_logical_exclusive_or(processor, &processor->state.accumulator, &data);
            break;
        case INSTRUCTION_ORA:
            nesl_processor_operation_logical_or(processor, &processor->state.accumulator, &data);
            break;
        default:
            break;
    }

    if(operand->page_cross) {
        ++processor->cycle;
    }
}

/**
 * @brief Execute no-op instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_no_operation(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    return;
}

/**
 * @brief Execute pull instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_pull(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case INSTRUCTION_PLA:
            processor->state.accumulator.low = nesl_processor_pull(processor);
            processor->state.status.negative = processor->state.accumulator.bit_7;
            processor->state.status.zero = !processor->state.accumulator.low;
            break;
        case INSTRUCTION_PLP:
            processor->state.status.raw = nesl_processor_pull(processor);
            processor->state.status.unused = true;
            break;
        default:
            break;
    }
}

/**
 * @brief Execute push instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_push(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case INSTRUCTION_PHA:
            nesl_processor_push(processor, processor->state.accumulator.low);
            break;
        case INSTRUCTION_PHP:
            nesl_processor_push(processor, processor->state.status.raw);
            break;
        default:
            break;
    }
}

/**
 * @brief Execute return instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_return(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case INSTRUCTION_RTI:
            processor->state.status.raw = nesl_processor_pull(processor);
            processor->state.status.unused = true;
            processor->state.program_counter.word = nesl_processor_pull_word(processor);
            break;
        case INSTRUCTION_RTS:
            processor->state.program_counter.word = nesl_processor_pull_word(processor) + 1;
            break;
        default:
            break;
    }
}

/**
 * @brief Execute rotate instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_rotate(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != OPERAND_ACCUMULATOR) {
        data.low = nesl_bus_read(BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case INSTRUCTION_ROL:
            nesl_processor_operation_rotate_left(processor, &data, NULL);
            break;
        case INSTRUCTION_ROR:
            nesl_processor_operation_rotate_right(processor, &data, NULL);
            break;
        default:
            break;
    }

    if(instruction->mode != OPERAND_ACCUMULATOR) {
        nesl_bus_write(BUS_PROCESSOR, operand->effective.word, data.low);
    } else {
        processor->state.accumulator.low = data.low;
    }
}

/**
 * @brief Execute set instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_set(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case INSTRUCTION_SEC:
            processor->state.status.carry = true;
            break;
        case INSTRUCTION_SED:
            processor->state.status.decimal = true;
            break;
        case INSTRUCTION_SEI:
            processor->state.status.interrupt_disable = true;
            break;
        default:
            break;
    }
}

/**
 * @brief Execute shift instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_shift(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = { .low = operand->data.low };

    if(instruction->mode != OPERAND_ACCUMULATOR) {
        data.low = nesl_bus_read(BUS_PROCESSOR, operand->effective.word);
    }

    switch(instruction->type) {
        case INSTRUCTION_ASL:
            nesl_processor_operation_shift_left(processor, &data, NULL);
            break;
        case INSTRUCTION_LSR:
            nesl_processor_operation_shift_right(processor, &data, NULL);
            break;
        default:
            break;
    }

    if(instruction->mode != OPERAND_ACCUMULATOR) {
        nesl_bus_write(BUS_PROCESSOR, operand->effective.word, data.low);
    } else {
        processor->state.accumulator.low = data.low;
    }
}

/**
 * @brief Execute store instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_store(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{
    nesl_register_t data = {};

    switch(instruction->type) {
        case INSTRUCTION_STA:
            data.low = processor->state.accumulator.low;
            break;
        case INSTRUCTION_STX:
            data.low = processor->state.index.x.low;
            break;
        case INSTRUCTION_STY:
            data.low = processor->state.index.y.low;
            break;
        default:
            break;
    }

    nesl_bus_write(BUS_PROCESSOR, operand->effective.word, data.low);
}

/**
 * @brief Execute transfer instruction.
 * @param processor Pointer to processor subsystem context
 * @param instruction Constant pointer to instruction context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_execute_transfer(nesl_processor_t *processor, const nesl_instruction_t *instruction, const nesl_operand_t *operand)
{

    switch(instruction->type) {
        case INSTRUCTION_TAX:
            nesl_processor_operation_transfer(processor, &processor->state.index.x, &processor->state.accumulator);
            break;
        case INSTRUCTION_TAY:
            nesl_processor_operation_transfer(processor, &processor->state.index.y, &processor->state.accumulator);
            break;
        case INSTRUCTION_TSX:
            nesl_processor_operation_transfer(processor, &processor->state.index.x, &processor->state.stack_pointer);
            break;
        case INSTRUCTION_TXA:
            nesl_processor_operation_transfer(processor, &processor->state.accumulator, &processor->state.index.x);
            break;
        case INSTRUCTION_TXS:
            nesl_processor_operation_transfer(processor, &processor->state.stack_pointer, &processor->state.index.x);
            break;
        case INSTRUCTION_TYA:
            nesl_processor_operation_transfer(processor, &processor->state.accumulator, &processor->state.index.y);
            break;
        default:
            break;
    }
}

/**
 * @brief Calcuate absolute addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_absolute(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch_word(processor);
    operand->effective.word = operand->data.word;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

/**
 * @brief Calcuate absolute-x addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_absolute_x(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch_word(processor);
    operand->effective.word = operand->data.word + processor->state.index.x.low;
    operand->indirect.word = 0;
    operand->page_cross = (operand->effective.high != operand->data.high);
}

/**
 * @brief Calcuate absolute-y addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_absolute_y(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch_word(processor);
    operand->effective.word = operand->data.word + processor->state.index.y.low;
    operand->indirect.word = 0;
    operand->page_cross = (operand->effective.high != operand->data.high);
}

/**
 * @brief Calcuate accumulator addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_accumulator(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = processor->state.accumulator.low;
    operand->effective.word = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

/**
 * @brief Calcuate immediate addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_immediate(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->effective.word = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

/**
 * @brief Calcuate implied addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_implied(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = 0;
    operand->effective.word = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

/**
 * @brief Calcuate indirect addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_indirect(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch_word(processor);
    operand->indirect.word = operand->data.word;

    if(operand->indirect.low == 0xFF) {
        operand->effective.low = nesl_bus_read(BUS_PROCESSOR, operand->indirect.word);
        operand->effective.high = nesl_bus_read(BUS_PROCESSOR, operand->indirect.word & 0xFF00);
    } else {
        operand->effective.word = nesl_processor_read_word(processor, operand->indirect.word);
    }

    operand->page_cross = false;
}

/**
 * @brief Calcuate indirect-x addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_indirect_x(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->indirect.low = (operand->data.word + processor->state.index.x.low);
    operand->indirect.high = 0;
    operand->effective.low = nesl_bus_read(BUS_PROCESSOR, operand->indirect.low);
    operand->effective.high = nesl_bus_read(BUS_PROCESSOR, (operand->indirect.low + 1) & 0xFF);
    operand->page_cross = false;
}

/**
 * @brief Calcuate indirect-y addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_indirect_y(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->indirect.low = nesl_bus_read(BUS_PROCESSOR, operand->data.low);
    operand->indirect.high = nesl_bus_read(BUS_PROCESSOR, (operand->data.low + 1) & 0xFF);
    operand->effective.word = operand->indirect.word + processor->state.index.y.low;
    operand->page_cross = (operand->effective.high != operand->indirect.high);
}

/**
 * @brief Calcuate relative addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_relative(nesl_processor_t *processor, nesl_operand_t *operand)

{
    operand->data.word = nesl_processor_fetch(processor);

    if(operand->data.bit_7) {
        operand->data.high = 0xFF;
    }

    operand->effective.word = processor->state.program_counter.word + operand->data.word;
    operand->indirect.word = 0;
    operand->page_cross = (operand->effective.high != processor->state.program_counter.high);
}

/**
 * @brief Calcuate zeropage addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_zeropage(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->effective.word = operand->data.word;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

/**
 * @brief Calcuate zeropage-x addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_zeropage_x(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->effective.low = operand->data.low + processor->state.index.x.low;
    operand->effective.high = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

/**
 * @brief Calcuate zeropage-y addressing mode operand.
 * @param processor Pointer to processor subsystem context
 * @param operand Constant pointer to operand context
 */
static void nesl_processor_operand_zeropage_y(nesl_processor_t *processor, nesl_operand_t *operand)
{
    operand->data.word = nesl_processor_fetch(processor);
    operand->effective.low = operand->data.low + processor->state.index.y.low;
    operand->effective.high = 0;
    operand->indirect.word = 0;
    operand->page_cross = false;
}

/**
 * @brief Execute instruction.
 * @param processor Pointer to processor subsystem context
 */
static void nesl_processor_instruction(nesl_processor_t *processor)
{
    static const nesl_processor_execute EXECUTE[] = {
        nesl_processor_execute_arithmetic, nesl_processor_execute_logical, nesl_processor_execute_shift, nesl_processor_execute_branch,
        nesl_processor_execute_branch, nesl_processor_execute_branch, nesl_processor_execute_bit, nesl_processor_execute_branch,
        nesl_processor_execute_branch, nesl_processor_execute_branch, nesl_processor_execute_breakpoint, nesl_processor_execute_branch,
        nesl_processor_execute_branch, nesl_processor_execute_clear, nesl_processor_execute_clear, nesl_processor_execute_clear,
        nesl_processor_execute_clear, nesl_processor_execute_compare, nesl_processor_execute_compare, nesl_processor_execute_compare,
        nesl_processor_execute_decrement, nesl_processor_execute_decrement, nesl_processor_execute_decrement, nesl_processor_execute_logical,
        nesl_processor_execute_increment, nesl_processor_execute_increment, nesl_processor_execute_increment, nesl_processor_execute_jump,
        nesl_processor_execute_jump, nesl_processor_execute_load, nesl_processor_execute_load, nesl_processor_execute_load,
        nesl_processor_execute_shift, nesl_processor_execute_no_operation, nesl_processor_execute_logical, nesl_processor_execute_push,
        nesl_processor_execute_push, nesl_processor_execute_pull, nesl_processor_execute_pull, nesl_processor_execute_rotate,
        nesl_processor_execute_rotate, nesl_processor_execute_return, nesl_processor_execute_return, nesl_processor_execute_arithmetic,
        nesl_processor_execute_set, nesl_processor_execute_set, nesl_processor_execute_set, nesl_processor_execute_store,
        nesl_processor_execute_store, nesl_processor_execute_store, nesl_processor_execute_transfer, nesl_processor_execute_transfer,
        nesl_processor_execute_transfer, nesl_processor_execute_transfer, nesl_processor_execute_transfer, nesl_processor_execute_transfer,
        nesl_processor_execute_no_operation,
        };

    static const nesl_instruction_t INSTRUCTION[] = {
        { INSTRUCTION_BRK, OPERAND_IMPLIED, 7 }, { INSTRUCTION_ORA, OPERAND_INDIRECT_X, 6 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_X, 8 },
        { INSTRUCTION_NOP, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_ORA, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_ASL, OPERAND_ZEROPAGE, 5 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE, 5 },
        { INSTRUCTION_PHP, OPERAND_IMPLIED, 3 }, { INSTRUCTION_ORA, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_ASL, OPERAND_ACCUMULATOR, 2 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 },
        { INSTRUCTION_NOP, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_ORA, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_ASL, OPERAND_ABSOLUTE, 6 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE, 6 },
        { INSTRUCTION_BPL, OPERAND_RELATIVE, 2 }, { INSTRUCTION_ORA, OPERAND_INDIRECT_Y, 5 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_Y, 8 },
        { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_ORA, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_ASL, OPERAND_ZEROPAGE_X, 6 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 6 },
        { INSTRUCTION_CLC, OPERAND_IMPLIED, 2 }, { INSTRUCTION_ORA, OPERAND_ABSOLUTE_Y, 4 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_Y, 7 },
        { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_ORA, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_ASL, OPERAND_ABSOLUTE_X, 7 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 7 },
        { INSTRUCTION_JSR, OPERAND_ABSOLUTE, 6 }, { INSTRUCTION_AND, OPERAND_INDIRECT_X, 6 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_X, 8 },
        { INSTRUCTION_BIT, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_AND, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_ROL, OPERAND_ZEROPAGE, 5 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE, 5 },
        { INSTRUCTION_PLP, OPERAND_IMPLIED, 4 }, { INSTRUCTION_AND, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_ROL, OPERAND_ACCUMULATOR, 2 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 },
        { INSTRUCTION_BIT, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_AND, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_ROL, OPERAND_ABSOLUTE, 6 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE, 6 },
        { INSTRUCTION_BMI, OPERAND_RELATIVE, 2 }, { INSTRUCTION_AND, OPERAND_INDIRECT_Y, 5 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_Y, 8 },
        { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_AND, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_ROL, OPERAND_ZEROPAGE_X, 6 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 6 },
        { INSTRUCTION_SEC, OPERAND_IMPLIED, 2 }, { INSTRUCTION_AND, OPERAND_ABSOLUTE_Y, 4 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_Y, 7 },
        { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_AND, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_ROL, OPERAND_ABSOLUTE_X, 7 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 7 },
        { INSTRUCTION_RTI, OPERAND_IMPLIED, 6 }, { INSTRUCTION_EOR, OPERAND_INDIRECT_X, 6 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_X, 8 },
        { INSTRUCTION_NOP, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_EOR, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_LSR, OPERAND_ZEROPAGE, 5 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE, 5 },
        { INSTRUCTION_PHA, OPERAND_IMPLIED, 3 }, { INSTRUCTION_EOR, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_LSR, OPERAND_ACCUMULATOR, 2 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 },
        { INSTRUCTION_JMP, OPERAND_ABSOLUTE, 3 }, { INSTRUCTION_EOR, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_LSR, OPERAND_ABSOLUTE, 6 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE, 6 },
        { INSTRUCTION_BVC, OPERAND_RELATIVE, 2 }, { INSTRUCTION_EOR, OPERAND_INDIRECT_Y, 5 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_Y, 8 },
        { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_EOR, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_LSR, OPERAND_ZEROPAGE_X, 6 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 6 },
        { INSTRUCTION_CLI, OPERAND_IMPLIED, 2 }, { INSTRUCTION_EOR, OPERAND_ABSOLUTE_Y, 4 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_Y, 7 },
        { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_EOR, OPERAND_ABSOLUTE_X, 4 },{ INSTRUCTION_LSR, OPERAND_ABSOLUTE_X, 7 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 7 },
        { INSTRUCTION_RTS, OPERAND_IMPLIED, 6 }, { INSTRUCTION_ADC, OPERAND_INDIRECT_X, 6 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_X, 8 },
        { INSTRUCTION_NOP, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_ADC, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_ROR, OPERAND_ZEROPAGE, 5 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE, 5 },
        { INSTRUCTION_PLA, OPERAND_IMPLIED, 4 }, { INSTRUCTION_ADC, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_ROR, OPERAND_ACCUMULATOR, 2 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 },
        { INSTRUCTION_JMP, OPERAND_INDIRECT, 5 }, { INSTRUCTION_ADC, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_ROR, OPERAND_ABSOLUTE, 6 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE, 6 },
        { INSTRUCTION_BVS, OPERAND_RELATIVE, 2 }, { INSTRUCTION_ADC, OPERAND_INDIRECT_Y, 5 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_Y, 8 },
        { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_ADC, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_ROR, OPERAND_ZEROPAGE_X, 6 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 6 },
        { INSTRUCTION_SEI, OPERAND_IMPLIED, 2 }, { INSTRUCTION_ADC, OPERAND_ABSOLUTE_Y, 4 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_Y, 7 },
        { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_ADC, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_ROR, OPERAND_ABSOLUTE_X, 7 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 7 },
        { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_STA, OPERAND_INDIRECT_X, 6 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_X, 6 },
        { INSTRUCTION_STY, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_STA, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_STX, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE, 3 },
        { INSTRUCTION_DEY, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_TXA, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 },
        { INSTRUCTION_STY, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_STA, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_STX, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE, 4 },
        { INSTRUCTION_BCC, OPERAND_RELATIVE, 2 }, { INSTRUCTION_STA, OPERAND_INDIRECT_Y, 6 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_Y, 6 },
        { INSTRUCTION_STY, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_STA, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_STX, OPERAND_ZEROPAGE_Y, 4 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE_Y, 4 },
        { INSTRUCTION_TYA, OPERAND_IMPLIED, 2 }, { INSTRUCTION_STA, OPERAND_ABSOLUTE_Y, 5 }, { INSTRUCTION_TXS, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_Y, 5 },
        { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 5 }, { INSTRUCTION_STA, OPERAND_ABSOLUTE_X, 5 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_Y, 5 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_Y, 5 },
        { INSTRUCTION_LDY, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_LDA, OPERAND_INDIRECT_X, 6 }, { INSTRUCTION_LDX, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_X, 6 },
        { INSTRUCTION_LDY, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_LDA, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_LDX, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE, 3 },
        { INSTRUCTION_TAY, OPERAND_IMPLIED, 2 }, { INSTRUCTION_LDA, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_TAX, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 },
        { INSTRUCTION_LDY, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_LDA, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_LDX, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE, 4 },
        { INSTRUCTION_BCS, OPERAND_RELATIVE, 2 }, { INSTRUCTION_LDA, OPERAND_INDIRECT_Y, 5 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_Y, 5 },
        { INSTRUCTION_LDY, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_LDA, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_LDX, OPERAND_ZEROPAGE_Y, 4 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE_Y, 4 },
        { INSTRUCTION_CLV, OPERAND_IMPLIED, 2 }, { INSTRUCTION_LDA, OPERAND_ABSOLUTE_Y, 4 }, { INSTRUCTION_TSX, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_Y, 4 },
        { INSTRUCTION_LDY, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_LDA, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_LDX, OPERAND_ABSOLUTE_Y, 4 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_Y, 4 },
        { INSTRUCTION_CPY, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_CMP, OPERAND_INDIRECT_X, 6 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_X, 8 },
        { INSTRUCTION_CPY, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_CMP, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_DEC, OPERAND_ZEROPAGE, 5 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE, 5 },
        { INSTRUCTION_INY, OPERAND_IMPLIED, 2 }, { INSTRUCTION_CMP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_DEX, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 },
        { INSTRUCTION_CPY, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_CMP, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_DEC, OPERAND_ABSOLUTE, 6 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE, 6 },
        { INSTRUCTION_BNE, OPERAND_RELATIVE, 2 }, { INSTRUCTION_CMP, OPERAND_INDIRECT_Y, 5 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_Y, 8 },
        { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_CMP, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_DEC, OPERAND_ZEROPAGE_X, 6 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 6 },
        { INSTRUCTION_CLD, OPERAND_IMPLIED, 2 }, { INSTRUCTION_CMP, OPERAND_ABSOLUTE_Y, 4 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_Y, 7 },
        { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_CMP, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_DEC, OPERAND_ABSOLUTE_X, 7 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 7 },
        { INSTRUCTION_CPX, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_SBC, OPERAND_INDIRECT_X, 6 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_X, 8 },
        { INSTRUCTION_CPX, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_SBC, OPERAND_ZEROPAGE, 3 }, { INSTRUCTION_INC, OPERAND_ZEROPAGE, 5 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE, 5 },
        { INSTRUCTION_INX, OPERAND_IMPLIED, 2 }, { INSTRUCTION_SBC, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 },
        { INSTRUCTION_CPX, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_SBC, OPERAND_ABSOLUTE, 4 }, { INSTRUCTION_INC, OPERAND_ABSOLUTE, 6 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE, 6 },
        { INSTRUCTION_BEQ, OPERAND_RELATIVE, 2 }, { INSTRUCTION_SBC, OPERAND_INDIRECT_Y, 5 }, { INSTRUCTION_NOP, OPERAND_IMPLIED, 2 }, { INSTRUCTION_NOP, OPERAND_INDIRECT_Y, 8 },
        { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_SBC, OPERAND_ZEROPAGE_X, 4 }, { INSTRUCTION_INC, OPERAND_ZEROPAGE_X, 6 }, { INSTRUCTION_NOP, OPERAND_ZEROPAGE_X, 6 },
        { INSTRUCTION_SED, OPERAND_IMPLIED, 2 }, { INSTRUCTION_SBC, OPERAND_ABSOLUTE_Y, 4 }, { INSTRUCTION_NOP, OPERAND_IMMEDIATE, 2 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_Y, 7 },
        { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_SBC, OPERAND_ABSOLUTE_X, 4 }, { INSTRUCTION_INC, OPERAND_ABSOLUTE_X, 7 }, { INSTRUCTION_NOP, OPERAND_ABSOLUTE_X, 7 },
        };

    static const nesl_processor_operand OPERAND[] = {
        nesl_processor_operand_absolute, nesl_processor_operand_absolute_x, nesl_processor_operand_absolute_y, nesl_processor_operand_accumulator,
        nesl_processor_operand_immediate, nesl_processor_operand_implied, nesl_processor_operand_indirect, nesl_processor_operand_indirect_x,
        nesl_processor_operand_indirect_y, nesl_processor_operand_relative, nesl_processor_operand_zeropage, nesl_processor_operand_zeropage_x,
        nesl_processor_operand_zeropage_y,
        };

    uint8_t opcode;
    nesl_operand_t operand = {};
    const nesl_instruction_t *instruction = &INSTRUCTION[(opcode = nesl_processor_fetch(processor))];

    processor->cycle = instruction->cycles;
    OPERAND[instruction->mode](processor, &operand);
    EXECUTE[instruction->type](processor, instruction, &operand);
}

/**
 * @brief Execute maskable interrupt (IRQ).
 * @param processor Pointer to processor subsystem context
 */
static void nesl_processor_interrupt_maskable(nesl_processor_t *processor)
{
    processor->interrupt.maskable = false;
    nesl_processor_push_word(processor, processor->state.program_counter.word);
    nesl_processor_push(processor, processor->state.status.raw);
    processor->state.program_counter.word = nesl_processor_read_word(processor, 0xFFFE);
    processor->state.status.interrupt_disable = true;
    processor->cycle = 7;
}

/**
 * @brief Execute non-maskable interrupt (NMI).
 * @param processor Pointer to processor subsystem context
 */
static void nesl_processor_interrupt_non_maskable(nesl_processor_t *processor)
{
    processor->interrupt.non_maskable = false;
    nesl_processor_push_word(processor, processor->state.program_counter.word);
    nesl_processor_push(processor, processor->state.status.raw);
    processor->state.program_counter.word = nesl_processor_read_word(processor, 0xFFFA);
    processor->state.status.interrupt_disable = true;
    processor->cycle = 7;
}

/**
 * @brief Execute transfer (DMA).
 * @param processor Pointer to processor subsystem context
 */
static void nesl_processor_transfer(nesl_processor_t *processor, uint64_t cycle)
{

    if(processor->interrupt.transfer_sync) {

        if(cycle % 2) {
            processor->interrupt.transfer_sync = false;
        }

        ++processor->cycle;
    } else {

        if(!(cycle % 2)) {
            processor->transfer.data = nesl_bus_read(BUS_PROCESSOR, processor->transfer.source.word++);
            ++processor->cycle;
        } else {
            nesl_bus_write(BUS_VIDEO_OAM, processor->transfer.destination.low++, processor->transfer.data);
            ++processor->cycle;

            if(processor->transfer.destination.low == 0x00) {
                processor->interrupt.transfer = false;
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

            if(processor->interrupt.transfer) {
                nesl_processor_transfer(processor, cycle);
            }

            if(!processor->interrupt.transfer) {

                if(processor->interrupt.non_maskable) {
                    nesl_processor_interrupt_non_maskable(processor);
                } else if(processor->interrupt.maskable
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

nesl_error_e nesl_processor_initialize(nesl_processor_t *processor)
{
    return nesl_processor_reset(processor);
}

nesl_error_e nesl_processor_interrupt(nesl_processor_t *processor, bool maskable)
{

    if(maskable) {
        processor->interrupt.maskable = true;
    } else {
        processor->interrupt.non_maskable = true;
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
