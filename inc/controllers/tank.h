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

#include <glib-2.0/glib.h>

#include <utils/utils.h>
#include <core/gpio.h>

#define TANK_LEVEL_PERCENT_DEFAULT  200
#define TANK_LEVEL_PERCENT_MAX      100
#define TANK_LEVEL_PERCENT_MIN      0

#define TANK_DB_FILE    "tank.db"

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

typedef struct {
    unsigned        percent;
    GpioPin         *gpio;
    bool            notify;
    bool            state;
} TankLevel;

typedef struct {
    char        name[SHORT_STR_LEN];
    GpioPin     *gpio[TANK_GPIO_MAX];
    GList       *levels;
    unsigned    level;
    bool        status;
    bool        pump;
    bool        valve;
} Tank;

/**
 * @brief Make new Tank Level object
 * 
 * @param percent Tank Level percentage
 * @param gpio Water level reedswitch pin
 * @param notify Send notify when level detected
 * 
 * @return TankLevel object
 */
TankLevel *TankLevelNew(unsigned percent, GpioPin *gpio, bool notify);

/**
 * @brief Make new Tank controller
 * 
 * @param name Tank's name
 * 
 * @return Tank object
 */
Tank *TankNew(const char *name);

/**
 * @brief Add new tank to list
 * 
 * @param tank Tank object
 */
void TankAdd(Tank *tank);

/**
 * @brief Get all tanks list
 * 
 * @return Tank controllers
 */
GList **TanksGet();

/**
 * @brief Get tank by name
 * 
 * @param name Tank controller name
 * 
 * @return Tank controller
 */
Tank *TankGet(const char *name);

/**
 * @brief Add new tank level
 * 
 * @param tank Tank controller
 * @param level Tank level object
 */
void TankLevelAdd(Tank *tank, TankLevel *level);

/**
 * @brief Set GPIO for Tank controller
 * 
 * @param tank Tank controller
 * @param id GPIO id
 * @param gpio GPIO object
 */
void TankGpioSet(Tank *tank, TankGpio id, GpioPin *gpio);

/**
 * @brief Set status for Tank
 * 
 * @param tank Tank controller
 * @param status Tank controller status
 * @param save Save status to DB
 * 
 * @return Tank controller
 */
bool TankStatusSet(Tank *tank, bool status, bool save);

/**
 * @brief Set pump status for Tank
 * 
 * @param tank Tank controller
 * @param status Tank controller pump status
 * 
 * @return Tank controller
 */
bool TankPumpSet(Tank *tank, bool status);

/**
 * @brief Set valve status for Tank
 * 
 * @param tank Tank controller
 * @param status Tank controller valve status
 * 
 * @return Tank controller
 */
bool TankValveSet(Tank *tank, bool status);

/**
 * @brief Get tank status of Tank controller
 * 
 * @param tank Tank controller
 * 
 * @return Tank controller status
 */
bool TankStatusGet(Tank *tank);

/**
 * @brief Start all tanks controllers
 * 
 * @return True/False as result of starting
 */
bool TankControllerStart();

#endif /* __TANK_H__ */