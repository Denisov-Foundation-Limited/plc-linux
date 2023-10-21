/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __GPIO_H__
#define __GPIO_H__

#include <stdbool.h>

#include <glib.h>

#include <utils/utils.h>

typedef enum {
    GPIO_TYPE_DIGITAL,
    GPIO_TYPE_ANALOG
} GpioType;

typedef enum {
    GPIO_MODE_INPUT,
    GPIO_MODE_OUTPUT
} GpioMode;

typedef enum {
    GPIO_PULL_NONE,
    GPIO_PULL_UP,
    GPIO_PULL_DOWN
} GpioPull;

typedef struct {
    char        name[GPIO_NAME_STR_LEN];
    GpioType    type;
    unsigned    pin;
    GpioMode    mode;
    GpioPull    pull;
} GpioPin;

bool GpioPinAdd(const GpioPin *pin, char *err);

GpioPin *GpioPinGet(const char *name);

GList **GpioPinsGet();

bool GpioPinRead(const GpioPin *pin);

int GpioPinReadA(const GpioPin *pin);

bool GpioPinWrite(const GpioPin *pin, bool state);

bool GpioPinWriteA(const GpioPin *pin, int value);

#endif /* __GPIO_H__ */
