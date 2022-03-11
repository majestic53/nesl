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
 * @file bus.h
 * @brief Common bus used by subsystems for communication.
 */

#ifndef NESL_BUS_H_
#define NESL_BUS_H_

#include <service.h>

/**
 * @enum nesl_bus_e
 * @brief Bus types.
 */
typedef enum {
    BUS_PROCESSOR = 0,      /*< Processor bus (16-bit) */
    BUS_VIDEO,              /*< Video bus (12-bit) */
    BUS_VIDEO_OAM,          /*< Video OAM bus (8-bit) */
    BUS_MAX,                /*< Maximum bus */
} nesl_bus_e;

/**
 * @enum nesl_interrupt_e
 * @brief Interrupt types.
 */
typedef enum {
    INTERRUPT_RESET = 0,    /*< Reset interrupt (RST) */
    INTERRUPT_NON_MASKABLE, /*< Non-maskable interrupt (NMI) */
    INTERRUPT_MASKABLE,     /*< Maskable interrupt (IRQ) */
    INTERRUPT_MAPPER,       /*< Mapper interrupt (A12) */
    INTERRUPT_MAX,
} nesl_interrupt_e;

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Cycle bus and subsystems through one cycle.
 * @return true if frame is complete, false otherwise
 */
bool nesl_bus_cycle(void);

/**
 * @brief Initialize bus and subsystems.
 * @param data Constant pointer to cartridge data
 * @param length Cartridge data length in bytes
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_bus_initialize(const void *data, int length);

/**
 * @brief Send bus interrupt to subsystems.
 * @param type Interrupt type
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e nesl_bus_interrupt(nesl_interrupt_e type);

/**
 * @brief Read byte from bus subsystems.
 * @param type Bus type
 * @param address Bus address
 * @return Bus data
 */
uint8_t nesl_bus_read(nesl_bus_e type, uint16_t address);

/**
 * @brief Uninitialize bus and subsystems.
 */
void nesl_bus_uninitialize(void);

/**
 * @brief Write byte to bus subsystems.
 * @param type Bus type
 * @param address Bus address
 * @param data Bus data
 */
void nesl_bus_write(nesl_bus_e type, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_BUS_H_ */
