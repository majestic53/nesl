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
 * @file NESL_processor.h
 * @brief Processor subsystem.
 */

#ifndef NESL_PROCESSOR_H_
#define NESL_PROCESSOR_H_

#include "../NESL_bus.h"

/**
 * @union nesl_processor_status_t
 * @brief Processor status register.
 */
typedef union {

    struct {
        uint8_t carry : 1;                  /*< Carry flag */
        uint8_t zero : 1;                   /*< Zero flag */
        uint8_t interrupt_disable : 1;      /*< Interrupt disable flag */
        uint8_t decimal : 1;                /*< Decimal flag */
        uint8_t breakpoint : 1;             /*< Breakpoint flag */
        uint8_t unused : 1;                 /*< Unused flag */
        uint8_t overflow : 1;               /*< Overflow flag */
        uint8_t negative : 1;               /*< Negative flag */
    };

    uint8_t raw;                            /*< Raw byte */
} nesl_processor_status_t;

/**
 * @struct nesl_processor_t
 * @brief Processor subsystem context.
 */
typedef struct {
    uint8_t cycle;                          /*< Remaining cycles */
    uint8_t ram[2 * 1024];                  /*< Program RAM buffer */

    union {

        struct {
            uint8_t transfer : 1;           /*< Transger start flag */
            uint8_t transfer_sync : 1;      /*< Transger sync flag */
            uint8_t non_maskable : 1;       /*< Non-maskable interrupt flag */
            uint8_t maskable : 1;           /*< Maskable interrupt flag */
        };

        uint8_t raw;                        /*< Raw byte */
    } interrupt;

    struct {
        nesl_register_t accumulator;        /*< Accumulator register */
        nesl_register_t program_counter;    /*< Program counter register */
        nesl_register_t stack_pointer;      /*< Stack pointer register */
        nesl_processor_status_t status;     /*< Status register */

        struct {
            nesl_register_t x;              /*< Index-x register */
            nesl_register_t y;              /*< Index-y register */
        } index;
    } state;

    struct {
        uint8_t data;                       /*< Data */
        nesl_register_t destination;        /*< Destination address */
        nesl_register_t source;             /*< Source address */
    } transfer;
} nesl_processor_t;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Cycle processor subsystem through one cycle.
 * @param processor Pointer to processor subsystem context
 * @param cycle Current cycle
 */
void NESL_ProcessorCycle(nesl_processor_t *processor, uint64_t cycle);

/**
 * @brief Initialize processor subsystem.
 * @param processor Pointer to processor subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_ProcessorInitialize(nesl_processor_t *processor);

/**
 * @brief Send processor subsystem interrupt.
 * @param processor Pointer to processor subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_ProcessorInterrupt(nesl_processor_t *processor, bool maskable);

/**
 * @brief Read byte from processor subsystem.
 * @param processor Pointer to processor subsystem context
 * @param address Processor address
 * @return Processor data
 */
uint8_t NESL_ProcessorRead(nesl_processor_t *processor, uint16_t address);

/**
 * @brief Reset processor subsystem.
 * @param processor Pointer to processor subsystem context
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_ProcessorReset(nesl_processor_t *processor);

/**
 * @brief Uninitialize processor subsystem.
 * @param processor Pointer to processor subsystem context
 */
void NESL_ProcessorUninitialize(nesl_processor_t *processor);

/**
 * @brief Write byte to processor subsystem.
 * @param processor Pointer to processor subsystem context
 * @param address Processor address
 * @param data Processor data
 */
void NESL_ProcessorWrite(nesl_processor_t *processor, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_PROCESSOR_H_ */
