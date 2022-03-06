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
 * @file NESL_define.h
 * @brief Common definitions and enumerations.
 */

#ifndef NESL_DEFINE_H_
#define NESL_DEFINE_H_

#include <errno.h>
#include <pthread.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/**
 * @brief Versioning macros.
 */
#define NESL_MAJOR 0                /*< Major version*/
#define NESL_MINOR 1                /*< Minor version*/
#define NESL_PATCH 40               /*< Patch version*/

/**
 * @enum nesl_bank_e
 * @brief Bank types used in the cartridge/mapper subsystems.
 */
typedef enum {
    NESL_BANK_CHARACTER_RAM = 0,    /*< Character RAM bank (CHR) */
    NESL_BANK_CHARACTER_ROM,        /*< Character ROM bank (CHR) */
    NESL_BANK_PROGRAM_RAM,          /*< Program RAM bank (PRG) */
    NESL_BANK_PROGRAM_ROM,          /*< Program ROM bank (PRG) */
    NESL_BANK_MAX,                  /*< Maximum bank */
} nesl_bank_e;

/**
 * @enum nesl_bus_e
 * @brief Bus types used in the bus/subsystems.
 */
typedef enum {
    NESL_BUS_PROCESSOR = 0,         /*< Processor bus (16-bit) */
    NESL_BUS_VIDEO,                 /*< Video bus (12-bit) */
    NESL_BUS_VIDEO_OAM,             /*< Video OAM bus (8-bit) */
    NESL_BUS_MAX,                   /*< Maximum bus */
} nesl_bus_e;

/**
 * @enum nesl_button_e
 * @brief Button types used in input subsystem.
 */
typedef enum {
    NESL_BUTTON_A = 0,              /*< A button */
    NESL_BUTTON_B,                  /*< B button */
    NESL_BUTTON_SELECT,             /*< Select button */
    NESL_BUTTON_START,              /*< Start button */
    NESL_BUTTON_UP,                 /*< D-pad up button */
    NESL_BUTTON_DOWN,               /*< D-pad down button */
    NESL_BUTTON_LEFT,               /*< D-pad left button */
    NESL_BUTTON_RIGHT,              /*< D-pad right button */
    NESL_BUTTON_MAX,                /*< Maximum button */
} nesl_button_e;

/**
 * @enum nesl_channel_e
 * @brief Channel types used in the audio subsystem square-wave synthesizer.
 */
typedef enum {
    NESL_CHANNEL_1 = 0,             /*< First square-wave channel */
    NESL_CHANNEL_2,                 /*< Second square-wave channel */
    NESL_CHANNEL_MAX,               /*< Maximum channel */
} nesl_channel_e;

/**
 * @enum nesl_controller_e
 * @brief Controller types used in the input subsystem.
 */
typedef enum {
    NESL_CONTROLLER_1 = 0,          /*< First controller */
    NESL_CONTROLLER_2,              /*< Second controller */
    NESL_CONTROLLER_MAX,            /*< Maximum controller */
} nesl_controller_e;

/**
 * @enum nesl_instruction_e
 * @brief Instruction types used in the processor subsystem.
 */
typedef enum {
    NESL_INSTRUCTION_ADC = 0,       /*< Add-carry instruction */
    NESL_INSTRUCTION_AND,           /*< Logical and instruction */
    NESL_INSTRUCTION_ASL,           /*< Arithmetic shift-left instruction */
    NESL_INSTRUCTION_BCC,           /*< Branch if carry-clear instruction */
    NESL_INSTRUCTION_BCS,           /*< Branch if carry-set instruction */
    NESL_INSTRUCTION_BEQ,           /*< Branch if equal instruction */
    NESL_INSTRUCTION_BIT,           /*< Bit instruction */
    NESL_INSTRUCTION_BMI,           /*< Branch if negative instruction */
    NESL_INSTRUCTION_BNE,           /*< Branch if not-equal instruction */
    NESL_INSTRUCTION_BPL,           /*< Branch if positive instruction */
    NESL_INSTRUCTION_BRK,           /*< Break instruction */
    NESL_INSTRUCTION_BVC,           /*< Branch if overflow-clear instruction */
    NESL_INSTRUCTION_BVS,           /*< Branch if overflow-set instruction */
    NESL_INSTRUCTION_CLC,           /*< Clear carry instruction */
    NESL_INSTRUCTION_CLD,           /*< Clear decimal instruction */
    NESL_INSTRUCTION_CLI,           /*< Clear interrupt disable instruction */
    NESL_INSTRUCTION_CLV,           /*< Clear overflow instruction */
    NESL_INSTRUCTION_CMP,           /*< Compare accumulator instruction */
    NESL_INSTRUCTION_CPX,           /*< Compare index-x instruction */
    NESL_INSTRUCTION_CPY,           /*< Compare index-y instruction */
    NESL_INSTRUCTION_DEC,           /*< Decrement accumulator instruction */
    NESL_INSTRUCTION_DEX,           /*< Decrement index-x instruction */
    NESL_INSTRUCTION_DEY,           /*< Decrement index-y instruction */
    NESL_INSTRUCTION_EOR,           /*< Logical exclusive-orinstruction */
    NESL_INSTRUCTION_INC,           /*< Decrement accumulator instruction */
    NESL_INSTRUCTION_INX,           /*< Decrement index-x instruction */
    NESL_INSTRUCTION_INY,           /*< Decrement index-y instruction */
    NESL_INSTRUCTION_JMP,           /*< Jump instruction */
    NESL_INSTRUCTION_JSR,           /*< Jump to subroutine instruction */
    NESL_INSTRUCTION_LDA,           /*< Load accumulator instruction */
    NESL_INSTRUCTION_LDX,           /*< Load index-x instruction */
    NESL_INSTRUCTION_LDY,           /*< Load index-y instruction */
    NESL_INSTRUCTION_LSR,           /*< Logical shift-right instruction */
    NESL_INSTRUCTION_NOP,           /*< No-operation instruction */
    NESL_INSTRUCTION_ORA,           /*< Logical or instruction */
    NESL_INSTRUCTION_PHA,           /*< Push accumulator instruction */
    NESL_INSTRUCTION_PHP,           /*< Push status instruction */
    NESL_INSTRUCTION_PLA,           /*< Pull accumulator instruction */
    NESL_INSTRUCTION_PLP,           /*< Pull status instruction */
    NESL_INSTRUCTION_ROL,           /*< Rotate left instruction */
    NESL_INSTRUCTION_ROR,           /*< Rotate right instruction */
    NESL_INSTRUCTION_RTI,           /*< Return from interrupt instruction */
    NESL_INSTRUCTION_RTS,           /*< Return from subroutine instruction */
    NESL_INSTRUCTION_SBC,           /*< Subtract carry instruction */
    NESL_INSTRUCTION_SEC,           /*< Set carry instruction */
    NESL_INSTRUCTION_SED,           /*< Set decimal instruction */
    NESL_INSTRUCTION_SEI,           /*< Set interrupt disable instruction */
    NESL_INSTRUCTION_STA,           /*< Store accumulator instruction */
    NESL_INSTRUCTION_STX,           /*< Store index-x instruction */
    NESL_INSTRUCTION_STY,           /*< Store index-y instruction */
    NESL_INSTRUCTION_TAX,           /*< Transfer accumulator to index-x instruction */
    NESL_INSTRUCTION_TAY,           /*< Transfer accumulator to index-y instruction */
    NESL_INSTRUCTION_TSX,           /*< Transfer stack to index-x instruction */
    NESL_INSTRUCTION_TXA,           /*< Transfer index-x to accumulator instruction */
    NESL_INSTRUCTION_TXS,           /*< Transfer index-x to stack instruction */
    NESL_INSTRUCTION_TYA,           /*< Transfer index-y to accumulator instruction */
    NESL_INSTRUCTION_XXX,           /*< Undocumented instruction */
} nesl_instruction_e;

/**
 * @enum nesl_interrupt_e
 * @brief Interrupt types used by bus/subsystems.
 */
typedef enum {
    NESL_INTERRUPT_RESET = 0,       /*< Reset interrupt (RST) */
    NESL_INTERRUPT_NON_MASKABLE,    /*< Non-maskable interrupt (NMI) */
    NESL_INTERRUPT_MASKABLE,        /*< Maskable interrupt (IRQ) */
    NESL_INTERRUPT_MAPPER,          /*< Mapper interrupt (A12) */
    NESL_INTERRUPT_MAX,
} nesl_interrupt_e;

/**
 * @enum nesl_mapper_e
 * @brief Mapper types used by the cartridge/mapper subsystems.
 */
typedef enum {
    NESL_MAPPER_0 = 0,              /*< Mapper 0 (NROM) */
    NESL_MAPPER_1,                  /*< Mapper 1 (MMC1) */
    NESL_MAPPER_2,                  /*< Mapper 2 (UxROM) */
    NESL_MAPPER_3,                  /*< Mapper 3 (CNROM) */
    NESL_MAPPER_4 = 4,              /*< Mapper 4 (MMC3) */
    NESL_MAPPER_30 = 30,            /*< Mapper 30 (UNROM) */
    NESL_MAPPER_66 = 66,            /*< Mapper 66 (GxROM) */
} nesl_mapper_e;

/**
 * @enum nesl_mirror_e
 * @brief Mirroring types used by the cartridge/mapper subsystems.
 */
typedef enum {
    NESL_MIRROR_HORIZONTAL = 0,     /*< Horizontal mirroring */
    NESL_MIRROR_VERTICAL,           /*< Vertical mirroring */
    NESL_MIRROR_ONE_LOW,            /*< One-Low mirroring */
    NESL_MIRROR_ONE_HIGH,           /*< One-High mirroring */
    NESL_MIRROR_MAX,
} nesl_mirror_e;

/**
 * @enum nesl_operand_e
 * @brief Operand addressing modes types used by the processor subsystem.
 */
typedef enum {
    NESL_OPERAND_ABSOLUTE = 0,      /*< Absolute (ABS) */
    NESL_OPERAND_ABSOLUTE_X,        /*< Absolute index-x (ABS+X) */
    NESL_OPERAND_ABSOLUTE_Y,        /*< Absolute index-y (ABS+Y) */
    NESL_OPERAND_ACCUMULATOR,       /*< Accumulator (ACC) */
    NESL_OPERAND_IMMEDIATE,         /*< Immediate (IMM) */
    NESL_OPERAND_IMPLIED,           /*< Implied (IMP) */
    NESL_OPERAND_INDIRECT,          /*< Indirect (IND) */
    NESL_OPERAND_INDIRECT_X,        /*< Indirect index-x (IND+X) */
    NESL_OPERAND_INDIRECT_Y,        /*< Indirect index-y (IND+Y) */
    NESL_OPERAND_RELATIVE,          /*< Relative (REL) */
    NESL_OPERAND_ZEROPAGE,          /*< Zeropage (ZP) */
    NESL_OPERAND_ZEROPAGE_X,        /*< Zeropage index-x (ZP+X) */
    NESL_OPERAND_ZEROPAGE_Y,        /*< Absolute index-y (ZP+Y) */
} nesl_operand_e;

/**
 * @enum nesl_port_e
 * @brief Video port types used by the video subsystem.
 */
typedef enum {
    NESL_PORT_CONTROL = 0,          /*< Control register */
    NESL_PORT_MASK,                 /*< Mask register */
    NESL_PORT_STATUS,               /*< Status register */
    NESL_PORT_OAM_ADDRESS,          /*< OAM address register */
    NESL_PORT_OAM_DATA,             /*< OAM data register */
    NESL_PORT_SCROLL,               /*< Scroll register */
    NESL_PORT_ADDRESS,              /*< Address register */
    NESL_PORT_DATA,                 /*< Data register */
} nesl_port_e;

/**
 * @struct nesl_register_t
 * @brief 16-bit register type used by processor/video subsystems.
 */
typedef union {

    struct {
        uint8_t bit_0 : 1;          /*< Bit-0 */
        uint8_t unused : 5;         /*< Unused bit */
        uint8_t bit_6 : 1;          /*< Bit-6 */
        uint8_t bit_7 : 1;          /*< Bit-7 */
    };

    struct {
        uint8_t low;                /*< Low byte */
        uint8_t high;               /*< High byte */
    };

    uint16_t word;                  /*< Word */
} nesl_register_t;

#endif /* NESL_DEFINE_H_ */
