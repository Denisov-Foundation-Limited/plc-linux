/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __WATERER_H__
#define __WATERER_H__

#include <stdbool.h>

#include <glib-2.0/glib.h>

#include <utils/utils.h>
#include <core/gpio.h>
#include <plc/plc.h>

#define WATERER_DB_FILE    "watering.db"

typedef enum {
    WATERER_GPIO_VALVE,
    WATERER_GPIO_STATUS_LED,
    WATERER_GPIO_STATUS_BUTTON,
    WATERER_GPIO_MAX
} WatererGpioType;

typedef struct {
    PlcTime time;
    bool    state;
    bool    notify;
} WateringTime;

typedef struct {
    char    name[SHORT_STR_LEN];
    GpioPin *gpio[WATERER_GPIO_MAX];
    GList   *times;
    bool    status;
    bool    valve;
} Waterer;

/**
 * @brief Make new Waterer object
 * 
 * @param name Name of waterer
 * 
 * @return Waterer object
 */
Waterer *WatererNew(const char *name);

/**
 * @brief Make new WateringTime object
 * 
 * @param time Time when switch Valve state
 * @param state Valve state
 * @param notify Notify of action
 * 
 * @return WateringTime object
 */
WateringTime *WateringTimeNew(PlcTime time, bool state, bool notify);

/**
 * @brief Set Waterer GPIO by type
 * 
 * @param wtr Waterer object
 */
void WatererGpioSet(Waterer *wtr, WatererGpioType type, GpioPin *gpio);

/**
 * @brief Add new tank to list
 * 
 * @param wtr Waterer object
 */
void WatererAdd(Waterer *wtr);

/**
 * @brief Add new tank to list
 * 
 * @param wtr Waterer object
 */
void WatererTimeAdd(Waterer *wtr, WateringTime *tm);

/**
 * @brief Get all tanks list
 * 
 * @return Waterer controllers
 */
GList **WaterersGet();

/**
 * @brief Set status for Waterer
 * 
 * @param tank Waterer controller
 * @param status Waterer controller status
 * @param save Save status to DB
 * 
 * @return Waterer controller
 */
bool WatererStatusSet(Waterer *wtr, bool status, bool save);

/**
 * @brief Set valve status for Waterer
 * 
 * @param wtr Waterer controller
 * @param status Waterer controller valve status
 * 
 * @return True/False as result of setting status
 */
bool WatererValveSet(Waterer *wtr, bool status);

/**
 * @brief Start all Waterer controllers
 * 
 * @return True/False as result of starting
 */
bool WatererControllerStart();

#endif /* __WATERER_H__ */
