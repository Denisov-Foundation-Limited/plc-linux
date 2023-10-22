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

#ifdef __ARM__
#include <wiringPi.h>
#endif

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

bool GpioInit()
{
#ifdef __ARM__
    if (wiringPiSetup() < 0) {
        return false;
    }
#endif
    return true;
}

bool GpioPinAdd(const GpioPin *pin, char *err)
{
#ifdef __ARM__
    switch (pin->mode) {
        case GPIO_MODE_INPUT:
            if (pinMode(pin->pin, INPUT) < 0) {
                return false;
            }
            break;

        case GPIO_MODE_OUTPUT:
            if (pinMode(pin->pin, OUTPUT) < 0) {
                return false;
            }
            break;
    }

    switch (pin->pull) {
        case GPIO_PULL_NONE:
            if (pullUpDnControl(pin->pin, PUD_OFF) < 0) {
                return false;
            }
            break;

        case GPIO_PULL_DOWN:
            if (pullUpDnControl(pin->pin, PUD_UP) < 0) {
                return false;
            }
            break;

        case GPIO_PULL_UP:
            if (pullUpDnControl(pin->pin, PUD_DOWN) < 0) {
                return false;
            }
            break;
    }
#endif

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
#ifdef __ARM__
    return digitalRead(pin->pin);
#endif
    return true;
}

int GpioPinReadA(const GpioPin *pin)
{
#ifdef __ARM__
    return analogRead(pin->pin);
#endif
    return 0;
}

bool GpioPinWrite(const GpioPin *pin, bool state)
{
#ifdef __ARM__
    if (digitalWrite(pin->pin, (state == true) ? HIGH : LOW) < 0) {
        return false;
    }
#endif
    return true;
}

bool GpioPinWriteA(const GpioPin *pin, int value)
{
#ifdef __ARM__
    if (analogWrite(pin->pin, value) < 0) {
        return false;
    }
#endif
    return true;
}
