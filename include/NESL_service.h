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

#ifndef NESL_SERVICE_H_
#define NESL_SERVICE_H_

#include "./NESL_common.h"

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

bool NESL_ServiceGetButton(int controller, int button);
int NESL_ServiceInit(const char *title, int fullscreen, int linear, int scale);
int NESL_ServicePoll(void);
int NESL_ServiceReset(void);
void NESL_ServiceSetPixel(uint8_t color, bool red, bool green, bool blue, uint8_t x, uint8_t y);
int NESL_ServiceShow(void);
void NESL_ServiceUninit(void);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* NESL_SERVICE_H_ */