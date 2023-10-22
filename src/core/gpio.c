/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <core/gpio.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static GList *pins = NULL;

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool GpioPinAdd(const GpioPin *pin, char *err)
{
    switch (pin->mode) {
        case GPIO_MODE_INPUT:
            break;

        case GPIO_MODE_OUTPUT:
            break;
    }

    switch (pin->pull) {
        case GPIO_PULL_NONE:
            break;

        case GPIO_PULL_DOWN:
            break;

        case GPIO_PULL_UP:
            break;
    }

    pins = g_list_append(pins, (void *)pin);
    return true;
}

GpioPin *GpioPinGet(const char *name)
{
    for (GList *p = pins; p != NULL; p = p->next) {
        GpioPin *pin = (GpioPin *)p->data;
        if (!strcmp(pin->name, name)) {
            return pin;
        }
    }
    return NULL;
}

GList **GpioPinsGet()
{
    return &pins;
}

bool GpioPinRead(const GpioPin *pin)
{
    return true;
}

int GpioPinReadA(const GpioPin *pin)
{
    return 0;
}

bool GpioPinWrite(const GpioPin *pin, bool state)
{
    return true;
}

bool GpioPinWriteA(const GpioPin *pin, int value)
{
    return true;
}
