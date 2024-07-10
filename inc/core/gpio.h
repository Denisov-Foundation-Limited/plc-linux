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

#include <glib-2.0/glib.h>

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
    char        name[SHORT_STR_LEN];
    GpioType    type;
    unsigned    pin;
    GpioMode    mode;
    GpioPull    pull;
} GpioPin;

/**
 * @brief Make new GPIO object
 * 
 * @param name GPIO name
 * @param type GPIO type
 * @param pin Pin number
 * @param mode GPIO mode
 * @param pull Pull up or down if needed
 * 
 * @return Gpio object
 */
GpioPin *GpioPinNew(const char *name, GpioType type, unsigned pin, GpioMode mode, GpioPull pull);

/**
 * @brief GPIO global initialization
 *
 * @return true/false as result of initialization GPIO
 */
bool GpioInit();

/**
 * @brief Add new GPIO
 * 
 * @param pin New pin
 * @param err Addition GPIO error output
 * 
 * @return true/false as result of addition new GPIO
 */
bool GpioPinAdd(const GpioPin *pin, char *err);

/**
 * @brief Get GPIO by name
 * 
 * @param name GPIO name
 * 
 * @return GPIO struct
 */
GpioPin *GpioPinGet(const char *name);

/**
 * @brief Get All GPIOs list
 * 
 * @return GPIO list
 */
GList **GpioPinsGet();

/**
 * @brief Read digital state from GPIO
 * 
 * @param pin GPIO pin
 * @param state Readed state
 * 
 * @return True/False as result of reading
 */
bool GpioPinRead(const GpioPin *pin, bool *state);

/**
 * @brief Read analog value from GPIO
 * 
 * @param pin GPIO pin
 * @param value Analog value
 * 
 * @return True/False as result of reading
 */
int GpioPinReadA(const GpioPin *pin, int *value);

/**
 * @brief Writing digital value to GPIO
 * 
 * @param pin GPIO pin
 * @param state Digital value
 * 
 * @return true/false as result of writing
 */
bool GpioPinWrite(const GpioPin *pin, bool state);

/**
 * @brief Writing analog value to GPIO
 * 
 * @param pin GPIO pin
 * @param value Analog value
 * 
 * @return true/false as result of writing
 */
void GpioPinWriteA(const GpioPin *pin, int value);

#endif /* __GPIO_H__ */
