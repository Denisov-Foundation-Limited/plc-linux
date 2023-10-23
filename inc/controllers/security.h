/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __SECURITY_CTRL_H__
#define __SECURITY_CTRL_H__

#include <stdbool.h>

#include <glib.h>

#include <utils/utils.h>
#include <core/gpio.h>

typedef enum {
    SECURITY_SENSOR_REED,
    SECURITY_SENSOR_MICRO_WAVE,
    SECURITY_SENSOR_PIR
} SecuritySensorType;

typedef enum {
    SECURITY_GPIO_STATUS_LED,
    SECURITY_GPIO_ALARM_LED,
    SECURITY_GPIO_BUZZER,
    SECURITY_GPIO_ALARM_RELAY,
    SECURITY_GPIO_MAX
} SecurityGpio;

typedef struct {
    char    value[SHORT_STR_LEN];
} SecurityKey;

typedef struct {
    char                name[SHORT_STR_LEN];
    SecuritySensorType  type;
    GpioPin             *gpio;
    bool                telegram;
    bool                sms;
    bool                alarm;
    bool                detected;
} SecuritySensor;

typedef struct {
    char    name[SHORT_STR_LEN];
    GList   *sensors;
    GList   *keys;
    GpioPin *gpio[SECURITY_GPIO_MAX];
    bool    status;
    bool    alarm;
    bool    last_alarm;
} SecurityController;

/**
 * @brief Add new Security controller to list
 * 
 * @param ctrl Current security controller
 */
void SecurityControllerAdd(const SecurityController *ctrl);

/**
 * @brief Get security controller by name
 * 
 * @param name Security controller name
 * 
 * @return Found security controller or NULL if not found
 */
SecurityController *SecurityControllerGet(const char *name);

/**
 * @brief Get all security controllers
 * 
 * @return List of security controllers
 */
GList **SecurityControllersGet();

/**
 * @brief Start all security controllers
 * 
 * @return true/false as result of controllers start
 */
bool SecurityControllersStart();

/**
 * @brief Switch status for security controller
 * 
 * @param ctrl Current security controller
 * @param status New security status
 * 
 * @return true/false as result of status switching
 */
bool SecurityStatusSet(SecurityController *ctrl, bool status);

/**
 * @brief Get current security status from security controller
 * 
 * @param ctrl Current security controller
 * 
 * @return Current security status for current controller
 */
bool SecurityStatusGet(const SecurityController *ctrl);

/**
 * @brief Add new iButton key for controller
 * 
 * @param ctrl Current security controller
 * @param key New security iButton key
 */
void SecurityKeyAdd(SecurityController *ctrl, const SecurityKey *key);

/**
 * @brief Add new security sensor for controller
 * 
 * @param ctrl Current security controller
 * @param sensor New security sensor 
 */
void SecuritySensorAdd(SecurityController *ctrl, const SecuritySensor *sensor);

/**
 * @brief Get security sensor from controller by name
 * 
 * @param ctrl Current security controller
 * @param name Security sensor name
 * 
 * @return Found security sensor or NULL if not found
 */
SecuritySensor *SecuritySensorGet(const SecurityController *ctrl, const char *name);

/**
 * @brief Get all security controller's sensors
 * 
 * @param ctrl Current security controller
 * 
 * @return List of sensors for current security controller
 */
GList **SecuritySensorsGet(SecurityController *ctrl);

#endif /* __SECURITY_CTRL_H__ */
