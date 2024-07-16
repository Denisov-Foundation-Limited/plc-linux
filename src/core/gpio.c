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
#include <wiringPiLite/wiringPi.h>
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

GpioPin *GpioPinNew(const char *name, GpioType type, unsigned pin, GpioMode mode, GpioPull pull)
{
    GpioPin *gpio = (GpioPin *)malloc(sizeof(GpioPin));

    strncpy(gpio->name, name, SHORT_STR_LEN);
    gpio->type = type;
    gpio->pin = pin;
    gpio->mode = mode;
    gpio->pull = pull;

    return gpio;
}

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
        if (!digitalWrite(pin->pin, LOW))
            return false;
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

bool GpioPinRead(const GpioPin *pin, bool *state)
{
    int     val;
    bool    ret;

    if (pin->pin == 0) {
        *state = false;
        return true;
    }

#ifdef __arm__
    if (!pinMode(pin->pin, INPUT)) {
        return false;
    }

    ret = digitalRead(pin->pin, &val);

    *state = (val == HIGH) ? true : false;

    return ret;
#endif

    *state = true;
    return true;
}

int GpioPinReadA(const GpioPin *pin, int *value)
{
    if (pin->pin == 0) {
        *value = 0;
        return true;
    }

#ifdef __arm__
    return analogRead(pin->pin, value);
#endif
    *value = 0;

    return true;
}

bool GpioPinWrite(const GpioPin *pin, bool state)
{
    if (pin->pin == 0) {
        return true;
    }
#ifdef __arm__
    if (!pinMode(pin->pin, OUTPUT)) {
        return false;
    }
    
    if (!digitalWrite(pin->pin, (state == true) ? HIGH : LOW))
        return false;
#endif
    return true;
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
