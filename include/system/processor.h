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

#ifndef NESL_PROCESSOR_H_
#define NESL_PROCESSOR_H_

#include "../bus.h"

enum {
    NESL_INSTRUCTION_ADC = 0,
    NESL_INSTRUCTION_AND,
    NESL_INSTRUCTION_ASL,
    NESL_INSTRUCTION_BCC,
    NESL_INSTRUCTION_BCS,
    NESL_INSTRUCTION_BEQ,
    NESL_INSTRUCTION_BIT,
    NESL_INSTRUCTION_BMI,
    NESL_INSTRUCTION_BNE,
    NESL_INSTRUCTION_BPL,
    NESL_INSTRUCTION_BRK,
    NESL_INSTRUCTION_BVC,
    NESL_INSTRUCTION_BVS,
    NESL_INSTRUCTION_CLC,
    NESL_INSTRUCTION_CLD,
    NESL_INSTRUCTION_CLI,
    NESL_INSTRUCTION_CLV,
    NESL_INSTRUCTION_CMP,
    NESL_INSTRUCTION_CPX,
    NESL_INSTRUCTION_CPY,
    NESL_INSTRUCTION_DEC,
    NESL_INSTRUCTION_DEX,
    NESL_INSTRUCTION_DEY,
    NESL_INSTRUCTION_EOR,
    NESL_INSTRUCTION_INC,
    NESL_INSTRUCTION_INX,
    NESL_INSTRUCTION_INY,
    NESL_INSTRUCTION_JMP,
    NESL_INSTRUCTION_JSR,
    NESL_INSTRUCTION_LDA,
    NESL_INSTRUCTION_LDX,
    NESL_INSTRUCTION_LDY,
    NESL_INSTRUCTION_LSR,
    NESL_INSTRUCTION_NOP,
    NESL_INSTRUCTION_ORA,
    NESL_INSTRUCTION_PHA,
    NESL_INSTRUCTION_PHP,
    NESL_INSTRUCTION_PLA,
    NESL_INSTRUCTION_PLP,
    NESL_INSTRUCTION_ROL,
    NESL_INSTRUCTION_ROR,
    NESL_INSTRUCTION_RTI,
    NESL_INSTRUCTION_RTS,
    NESL_INSTRUCTION_SBC,
    NESL_INSTRUCTION_SEC,
    NESL_INSTRUCTION_SED,
    NESL_INSTRUCTION_SEI,
    NESL_INSTRUCTION_STA,
    NESL_INSTRUCTION_STX,
    NESL_INSTRUCTION_STY,
    NESL_INSTRUCTION_TAX,
    NESL_INSTRUCTION_TAY,
    NESL_INSTRUCTION_TSX,
    NESL_INSTRUCTION_TXA,
    NESL_INSTRUCTION_TXS,
    NESL_INSTRUCTION_TYA,
    NESL_INSTRUCTION_XXX,
};

enum {
    NESL_OPERAND_ABSOLUTE = 0,
    NESL_OPERAND_ABSOLUTE_X,
    NESL_OPERAND_ABSOLUTE_Y,
    NESL_OPERAND_ACCUMULATOR,
    NESL_OPERAND_IMMEDIATE,
    NESL_OPERAND_IMPLIED,
    NESL_OPERAND_INDIRECT,
    NESL_OPERAND_INDIRECT_X,
    NESL_OPERAND_INDIRECT_Y,
    NESL_OPERAND_RELATIVE,
    NESL_OPERAND_ZEROPAGE,
    NESL_OPERAND_ZEROPAGE_X,
    NESL_OPERAND_ZEROPAGE_Y,
};

typedef union {

    struct {
        uint8_t carry : 1;
        uint8_t zero : 1;
        uint8_t interrupt_disable : 1;
        uint8_t decimal : 1;
        uint8_t breakpoint : 1;
        uint8_t unused : 1;
        uint8_t overflow : 1;
        uint8_t negative : 1;
    };

    uint8_t raw;
} nesl_processor_status_t;

typedef struct {
    uint8_t ram[2 * 1024];
    uint8_t cycle;

    union {

        struct {
            uint8_t transfer : 1;
            uint8_t transfer_sync : 1;
            uint8_t non_maskable : 1;
            uint8_t maskable : 1;
        };

        uint8_t raw;
    } event;

    struct {
        nesl_register_t accumulator;
        nesl_register_t program_counter;
        nesl_register_t stack_pointer;
        nesl_processor_status_t status;

        struct {
            nesl_register_t x;
            nesl_register_t y;
        } index;
    } state;

    struct {
        uint8_t data;
        nesl_register_t destination;
        nesl_register_t source;
    } transfer;
} nesl_processor_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

void nesl_processor_cycle(nesl_processor_t *processor, uint64_t cycle);

int nesl_processor_initialize(nesl_processor_t *processor);

int nesl_processor_interrupt(nesl_processor_t *processor, bool maskable);

uint8_t nesl_processor_read(nesl_processor_t *processor, uint16_t address);

int nesl_processor_reset(nesl_processor_t *processor);

void nesl_processor_uninitialize(nesl_processor_t *processor);

void nesl_processor_write(nesl_processor_t *processor, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_PROCESSOR_H_ */
