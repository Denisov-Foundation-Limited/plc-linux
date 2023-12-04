/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __MENU_H__
#define __MENU_H__

#include <stdbool.h>

#include <glib-2.0/glib.h>

#include <core/gpio.h>
#include <utils/utils.h>
#include <controllers/meteo.h>
#include <core/lcd.h>
#include <controllers/tank.h>

typedef enum {
    MENU_GPIO_UP,
    MENU_GPIO_MIDDLE,
    MENU_GPIO_DOWN,
    MENU_GPIO_MAX
} MenuGpioType;

typedef enum {
    MENU_CTRL_METEO,
    MENU_CTRL_TIME,
    MENU_CTRL_TANK
} MenuController;

typedef enum {
    MENU_TIME_NOW
} MenuTimeType;

typedef struct {
    MeteoSensor  *sensor;
} MenuMeteoCtrl;

typedef struct {
    MenuTimeType    type;
} MenuTimeCtrl;

typedef struct {
    Tank    *tank;
} MenuTankCtrl;

typedef struct {
    unsigned        row;
    unsigned        col;
    char            alias[SHORT_STR_LEN];
    MenuController  ctrl;
    MenuMeteoCtrl   meteo;
    MenuTimeCtrl    time;
    MenuTankCtrl    tank;
} MenuValue;

typedef struct {
    char    name[SHORT_STR_LEN];
    GList   *values;
} MenuLevel;

/**
 * @brief Set menu GPIO by type
 *
 * @param type GPIO type
 * @param gpio GPIO object
 */
void MenuGpioSet(MenuGpioType type, GpioPin *gpio);

/**
 * @brief Set menu LCD object
 *
 * @param lcd LCD object
 */
void MenuLcdSet(LCD *lcd);

/**
 * @brief Make new menu level
 *
 * @param name Name of menu level
 *
 * @return MenuLevel object
 */
MenuLevel *MenuLevelNew(const char *name);

/**
 * @brief Make new menu level value
 *
 * @param row LCD row of value
 * @param col LCD column of value
 * @param alias LCD value alias
 * @param ctrl Menu controller type
 *
 * @return MenuValue object
 */
MenuValue *MenuValueNew(unsigned row, unsigned col, const char *alias, MenuController ctrl);

/**
 * @brief Add new menu level
 *
 * @param level New menu level
 */
void MenuLevelAdd(MenuLevel *level);

/**
 * @brief Add new menu value
 *
 * @param level Menu level object
 * @param value Menu value object
 */
void MenuValueAdd(MenuLevel *level, MenuValue *value);

/**
 * @brief Start menu module
 *
 * @return True/False as result of starting
 */
bool MenuStart();

#endif /* __MENU_H__ */
