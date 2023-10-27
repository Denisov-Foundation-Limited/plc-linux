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

#ifdef __arm__
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
#ifdef __arm__
    if (wiringPiSetup() < 0) {
        return false;
    }
#endif
    return true;
}

bool GpioPinAdd(const GpioPin *pin, char *err)
{
#ifdef __arm__
    switch (pin->mode) {
        case GPIO_MODE_INPUT:
            pinMode(pin->pin, INPUT);
            break;

        case GPIO_MODE_OUTPUT:
            pinMode(pin->pin, OUTPUT);
            break;
    }

    switch (pin->pull) {
        case GPIO_PULL_NONE:
            pullUpDnControl(pin->pin, PUD_OFF);
            break;

        case GPIO_PULL_DOWN:
            pullUpDnControl(pin->pin, PUD_UP);
            break;

        case GPIO_PULL_UP:
            pullUpDnControl(pin->pin, PUD_DOWN);
            break;
    }

    if (pin->mode == GPIO_MODE_OUTPUT) {
        digitalWrite(pin->pin, LOW);
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
    if (pin->pin == 0) {
        return 0;
    }
#ifdef __arm__
    return digitalRead(pin->pin);
#endif
    return true;
}

int GpioPinReadA(const GpioPin *pin)
{
    if (pin->pin == 0) {
        return 0;
    }
#ifdef __arm__
    return analogRead(pin->pin);
#endif
    return 0;
}

void GpioPinWrite(const GpioPin *pin, bool state)
{
    if (pin->pin == 0) {
        return;
    }
#ifdef __arm__
    digitalWrite(pin->pin, (state == true) ? HIGH : LOW);
#endif
}

void GpioPinWriteA(const GpioPin *pin, int value)
{
    if (pin->pin == 0) {
        return;
    }
#ifdef __arm__
    analogWrite(pin->pin, value);
#endif
}
