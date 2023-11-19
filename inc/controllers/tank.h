/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __TANK_H__
#define __TANK_H__

#include <stdbool.h>

#include <utils/utils.h>
#include <core/gpio.h>

#define TANK_LEVEL_DEFAULT  200

typedef enum {
    TANK_GPIO_VALVE,
    TANK_GPIO_PUMP,
    TANK_GPIO_STATUS_LED,
    TANK_GPIO_STATUS_BUTTON,
    TANK_GPIO_MAX
} TankGpio;

typedef enum {
    TANK_LEVEL_LOW,
    TANK_LEVEL_MID,
    TANK_LEVEL_MAX
} TankLevelType;

typedef enum {
    TANK_NOTIFY_ZERO,
    TANK_NOTIFY_MAX
} TankNotify;

typedef struct {
    TankLevelType   type;
    unsigned        percent;
    GpioPin         *gpio;
    bool            notify;
    bool            state;
} TankLevel;

typedef struct {
    char        name[SHORT_STR_LEN];
    GpioPin     *gpio[TANK_GPIO_MAX];
    bool        notify[TANK_NOTIFY_MAX];
    GList       *levels;
    unsigned    level;
    bool        status;
} Tank;

TankLevel *TankLevelNew(TankLevelType type, unsigned percent, GpioPin *gpio, bool notify);

Tank *TankNew(const char *name);

void TankNotifySet(Tank *tank, TankNotify id, bool status);

void TankAdd(Tank *tank);

void TankLevelAdd(Tank *tank, TankLevel *level);

void TankGpioSet(Tank *tank, TankGpio id, GpioPin *gpio);

bool TankStatusSet(Tank *tank, bool status, bool save);

bool TankStatusGet(Tank *tank);

bool TankControllerStart();

#endif /* __TANK_H__ */