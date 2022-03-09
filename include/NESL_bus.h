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
 * @file NESL_bus.h
 * @brief Common bus used by subsystems for communication.
 */

#ifndef NESL_BUS_H_
#define NESL_BUS_H_

#include "./NESL_common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

/**
 * @brief Cycle bus and subsystems through one cycle.
 * @return true if frame is complete, false otherwise
 */
bool NESL_BusCycle(void);

/**
 * @brief Initialize bus and subsystems.
 * @param data Constant pointer to cartridge data
 * @param length Cartridge data length in bytes
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_BusInit(const void *data, int length);

/**
 * @brief Send bus interrupt to subsystems.
 * @param type Interrupt type
 * @return NESL_FAILURE on failure, NESL_SUCCESS otherwise
 */
nesl_error_e NESL_BusInterrupt(nesl_interrupt_e type);

/**
 * @brief Read byte from bus subsystems.
 * @param type Bus type
 * @param address Bus address
 * @return Bus data
 */
uint8_t NESL_BusRead(nesl_bus_e type, uint16_t address);

/**
 * @brief Uninitialize bus and subsystems.
 */
void NESL_BusUninit(void);

/**
 * @brief Write byte to bus subsystems.
 * @param type Bus type
 * @param address Bus address
 * @param data Bus data
 */
void NESL_BusWrite(nesl_bus_e type, uint16_t address, uint8_t data);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_BUS_H_ */
