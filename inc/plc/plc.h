/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __PLC_H__
#define __PLC_H__

#include <stdbool.h>

#include <core/gpio.h>

typedef enum {
    PLC_ALARM_SECURITY  = 0x1,
    PLC_ALARM_TANK      = 0x2
} PlcAlarmType;

typedef enum {
    PLC_GPIO_ALARM_LED,
    PLC_GPIO_BUZZER,
    PLC_GPIO_MAX
} PlcGpioType;

typedef enum {
    PLC_BUZZER_SECURITY_ENTER,
    PLC_BUZZER_SECURITY_EXIT,
    PLC_BUZZER_LOOP,
    PLC_BUZZER_TANK_EMPTY
} PlcBuzzerType;

typedef enum {
    PLC_TIME_LINUX,
    PLC_TIME_DS3231
} PlcTimeType;

typedef struct {
    unsigned sec;
    unsigned min;
    unsigned hour;
    unsigned day;
    unsigned dow;
    unsigned month;
    unsigned year;
} PlcTime;

/**
 * @brief Set type of PLC Time
 *
 * @param type Time type
 */
void PlcTimeTypeSet(PlcTimeType type);

/**
 * @brief Get current time
 *
 * @param time Current time
 */
bool PlcTimeGet(PlcTime *time);

/**
 * @brief Set GPIO by type
 *
 * @param type GPIO type
 * @param gpio GPIO object
 */
void PlcGpioSet(PlcGpioType type, GpioPin *gpio);

/**
 * @brief Set alarm LED status
 *
 * @param type Alarm Type
 * @param status Alarm status
 */
void PlcAlarmSet(PlcAlarmType type, bool status);

/**
 * @brief Set Buzzer sound
 *
 * @param type Buzzer sound type
 * @param status Buzzer sttaus
 */
void PlcBuzzerRun(PlcBuzzerType type, bool status);

/**
 * @brief Stop all buzzer sounds
 */
void PlcBuzzerStop();

/**
 * @brief Start general controller functions
 */
bool PlcStart();

#endif /* __PLC_H__ */
