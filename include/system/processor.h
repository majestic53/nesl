/*
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

/*!
 * @file processor.h
 * @brief Processor subsystem.
 */

#ifndef NESL_PROCESSOR_H_
#define NESL_PROCESSOR_H_

#include <bus.h>

/*!
 * @enum nesl_instruction_e
 * @brief Instruction types.
 */
typedef enum {
    INSTRUCTION_ADC = 0,                    /*!< Add-carry instruction */
    INSTRUCTION_AND,                        /*!< Logical and instruction */
    INSTRUCTION_ASL,                        /*!< Arithmetic shift-left instruction */
    INSTRUCTION_BCC,                        /*!< Branch if carry-clear instruction */
    INSTRUCTION_BCS,                        /*!< Branch if carry-set instruction */
    INSTRUCTION_BEQ,                        /*!< Branch if equal instruction */
    INSTRUCTION_BIT,                        /*!< Bit instruction */
    INSTRUCTION_BMI,                        /*!< Branch if negative instruction */
    INSTRUCTION_BNE,                        /*!< Branch if not-equal instruction */
    INSTRUCTION_BPL,                        /*!< Branch if positive instruction */
    INSTRUCTION_BRK,                        /*!< Break instruction */
    INSTRUCTION_BVC,                        /*!< Branch if overflow-clear instruction */
    INSTRUCTION_BVS,                        /*!< Branch if overflow-set instruction */
    INSTRUCTION_CLC,                        /*!< Clear carry instruction */
    INSTRUCTION_CLD,                        /*!< Clear decimal instruction */
    INSTRUCTION_CLI,                        /*!< Clear interrupt disable instruction */
    INSTRUCTION_CLV,                        /*!< Clear overflow instruction */
    INSTRUCTION_CMP,                        /*!< Compare accumulator instruction */
    INSTRUCTION_CPX,                        /*!< Compare index-x instruction */
    INSTRUCTION_CPY,                        /*!< Compare index-y instruction */
    INSTRUCTION_DEC,                        /*!< Decrement accumulator instruction */
    INSTRUCTION_DEX,                        /*!< Decrement index-x instruction */
    INSTRUCTION_DEY,                        /*!< Decrement index-y instruction */
    INSTRUCTION_EOR,                        /*!< Logical exclusive-orinstruction */
    INSTRUCTION_INC,                        /*!< Decrement accumulator instruction */
    INSTRUCTION_INX,                        /*!< Decrement index-x instruction */
    INSTRUCTION_INY,                        /*!< Decrement index-y instruction */
    INSTRUCTION_JMP,                        /*!< Jump instruction */
    INSTRUCTION_JSR,                        /*!< Jump to subroutine instruction */
    INSTRUCTION_LDA,                        /*!< Load accumulator instruction */
    INSTRUCTION_LDX,                        /*!< Load index-x instruction */
    INSTRUCTION_LDY,                        /*!< Load index-y instruction */
    INSTRUCTION_LSR,                        /*!< Logical shift-right instruction */
    INSTRUCTION_NOP,                        /*!< No-operation instruction */
    INSTRUCTION_ORA,                        /*!< Logical or instruction */
    INSTRUCTION_PHA,                        /*!< Push accumulator instruction */
    INSTRUCTION_PHP,                        /*!< Push status instruction */
    INSTRUCTION_PLA,                        /*!< Pull accumulator instruction */
    INSTRUCTION_PLP,                        /*!< Pull status instruction */
    INSTRUCTION_ROL,                        /*!< Rotate left instruction */
    INSTRUCTION_ROR,                        /*!< Rotate right instruction */
    INSTRUCTION_RTI,                        /*!< Return from interrupt instruction */
    INSTRUCTION_RTS,                        /*!< Return from subroutine instruction */
    INSTRUCTION_SBC,                        /*!< Subtract carry instruction */
    INSTRUCTION_SEC,                        /*!< Set carry instruction */
    INSTRUCTION_SED,                        /*!< Set decimal instruction */
    INSTRUCTION_SEI,                        /*!< Set interrupt disable instruction */
    INSTRUCTION_STA,                        /*!< Store accumulator instruction */
    INSTRUCTION_STX,                        /*!< Store index-x instruction */
    INSTRUCTION_STY,                        /*!< Store index-y instruction */
    INSTRUCTION_TAX,                        /*!< Transfer accumulator to index-x instruction */
    INSTRUCTION_TAY,                        /*!< Transfer accumulator to index-y instruction */
    INSTRUCTION_TSX,                        /*!< Transfer stack to index-x instruction */
    INSTRUCTION_TXA,                        /*!< Transfer index-x to accumulator instruction */
    INSTRUCTION_TXS,                        /*!< Transfer index-x to stack instruction */
    INSTRUCTION_TYA,                        /*!< Transfer index-y to accumulator instruction */
    INSTRUCTION_XXX,                        /*!< Undocumented instruction */
} nesl_instruction_e;

/*!
 * @enum nesl_operand_e
 * @brief Operand addressing mode types.
 */
typedef enum {
    OPERAND_ABSOLUTE = 0,                   /*!< Absolute (ABS) */
    OPERAND_ABSOLUTE_X,                     /*!< Absolute index-x (ABS+X) */
    OPERAND_ABSOLUTE_Y,                     /*!< Absolute index-y (ABS+Y) */
    OPERAND_ACCUMULATOR,                    /*!< Accumulator (ACC) */
    OPERAND_IMMEDIATE,                      /*!< Immediate (IMM) */
    OPERAND_IMPLIED,                        /*!< Implied (IMP) */
    OPERAND_INDIRECT,                       /*!< Indirect (IND) */
    OPERAND_INDIRECT_X,                     /*!< Indirect index-x (IND+X) */
    OPERAND_INDIRECT_Y,                     /*!< Indirect index-y (IND+Y) */
    OPERAND_RELATIVE,                       /*!< Relative (REL) */
    OPERAND_ZEROPAGE,                       /*!< Zeropage (ZP) */
    OPERAND_ZEROPAGE_X,                     /*!< Zeropage index-x (ZP+X) */
    OPERAND_ZEROPAGE_Y,                     /*!< Absolute index-y (ZP+Y) */
} nesl_operand_e;

/*!
 * @union nesl_processor_status_t
 * @brief Processor status register.
 */
typedef union {

    struct {
        uint8_t carry : 1;                  /*!< Carry flag */
        uint8_t zero : 1;                   /*!< Zero flag */
        uint8_t interrupt_disable : 1;      /*!< Interrupt disable flag */
        uint8_t decimal : 1;                /*!< Decimal flag */
        uint8_t breakpoint : 1;             /*!< Breakpoint flag */
        uint8_t unused : 1;                 /*!< Unused flag */
        uint8_t overflow : 1;               /*!< Overflow flag */
        uint8_t negative : 1;               /*!< Negative flag */
    };

    uint8_t raw;                            /*!< Raw byte */
} nesl_processor_status_t;

/*!
 * @struct nesl_processor_t
 * @brief Processor subsystem context.
 */
typedef struct {
    uint8_t cycle;                          /*!< Remaining cycles */
    uint8_t ram[2 * 1024];                  /*!< Program RAM buffer */

    union {

        struct {
            uint8_t transfer : 1;           /*!< Transger start flag */
            uint8_t transfer_sync : 1;      /*!< Transger sync flag */
            uint8_t non_maskable : 1;       /*!< Non-maskable interrupt flag */
            uint8_t maskable : 1;           /*!< Maskable interrupt flag */
        };

        uint8_t raw;                        /*!< Raw byte */
    } interrupt;

    struct {
        nesl_register_t accumulator;        /*!< Accumulator register */
        nesl_register_t program_counter;    /*!< Program counter register */
        nesl_register_t stack_pointer;      /*!< Stack pointer register */
        nesl_processor_status_t status;     /*!< Status register */

        struct {
            nesl_register_t x;              /*!< Index-x register */
            nesl_register_t y;              /*!< Index-y register */
        } index;
    } state;

    struct {
        uint8_t data;                       /*!< Data */
        nesl_register_t destination;        /*!< Destination address */
        nesl_register_t source;             /*!< Source address */
    } transfer;
} nesl_processor_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/*!
 * @brief Cycle processor subsystem through one cycle.
 * @param[in,out] processor Pointer to processor subsystem context
 * @param[in] cycle Current cycle
 */
void nesl_processor_cycle(nesl_processor_t *processor, uint64_t cycle);

/*!
 * @brief Initialize processor subsystem.
 * @param[in,out] processor Pointer to processor subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_processor_initialize(nesl_processor_t *processor);

/*!
 * @brief Send processor subsystem interrupt.
 * @param[in,out] processor Pointer to processor subsystem context
 * @param[in] maskable Maskable interrupt flag
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_processor_interrupt(nesl_processor_t *processor, bool maskable);

/*!
 * @brief Read byte from processor subsystem.
 * @param[in,out] processor Pointer to processor subsystem context
 * @param[in] address Processor address
 * @return Processor data
 */
uint8_t nesl_processor_read(nesl_processor_t *processor, uint16_t address);

/*!
 * @brief Reset processor subsystem.
 * @param[in,out] processor Pointer to processor subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_processor_reset(nesl_processor_t *processor);

/*!
 * @brief Uninitialize processor subsystem.
 * @param[in,out] processor Pointer to processor subsystem context
 */
void nesl_processor_uninitialize(nesl_processor_t *processor);

/*!
 * @brief Write byte to processor subsystem.
 * @param[in,out] processor Pointer to processor subsystem context
 * @param[in] address Processor address
 * @param[in] data Processor data
 */
void nesl_processor_write(nesl_processor_t *processor, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_PROCESSOR_H_ */
