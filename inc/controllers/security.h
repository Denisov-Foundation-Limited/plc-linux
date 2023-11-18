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

#include <glib-2.0/glib.h>

#include <utils/utils.h>
#include <core/gpio.h>

#define SECURITY_DB_FILE    "security.db"

#define SECURITY_DETECTED_TIME_MAX_SEC  3
#define SECURITY_SENSOR_TIME_MAX_SEC    60

typedef enum {
    SECURITY_SAVE_TYPE_STATUS,
    SECURITY_SAVE_TYPE_ALARM
} SecurityStatusType;

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

typedef enum {
    SECURITY_SCENARIO_IN,
    SECURITY_SCENARIO_OUT
} SecurityScenarioType;

typedef enum {
    SECURITY_CTRL_SOCKET
} SecurityScenarioCtrl;

typedef enum {
    SECURITY_SOUND_ENTER,
    SECURITY_SOUND_EXIT,
    SECURITY_SOUND_ALARM,
    SECURITY_SOUND_MAX
} SecuritySound;

typedef struct {
    char    name[SHORT_STR_LEN];
    char    id[SHORT_STR_LEN];
} SecurityKey;

typedef struct {
    char                name[SHORT_STR_LEN];
    SecuritySensorType  type;
    GpioPin             *gpio;
    bool                telegram;
    bool                sms;
    bool                alarm;
    bool                detected;
    unsigned            counter;
} SecuritySensor;

typedef struct {
    char    name[SHORT_STR_LEN];
    bool    status;
} SecurityScenarioSocket;

typedef struct {
    SecurityScenarioType    type;
    SecurityScenarioCtrl    ctrl;
    SecurityScenarioSocket  socket;
} SecurityScenario;

/**
 * @brief Set security sound status
 * 
 * @param sound Sound type
 * @param status Enabled or disabled status
 */
void SecuritySoundSet(SecuritySound sound, bool status);

/**
 * @brief Add scenario for in, out or other human actions
 * 
 * @param scenario Actions
 */
void SecurityScenarioAdd(SecurityScenario *scenario);

/**
 * @brief Check security key for all controllers
 * 
 * @param key Security key id
 * 
 * @return True if key found for controller, False if not found
 */
bool SecurityKeyCheck(const char *key);

/**
 * @brief Set gpio by ID
 * 
 * @param id Security GPIO uniq ID
 * @param gpio GpioPin pointer
 */
void SecurityGpioSet(SecurityGpio id, GpioPin *gpio);

/**
 * @brief Start all security controllers
 * 
 * @return true/false as result of controllers start
 */
bool SecurityControllerStart();

/**
 * @brief Switch status for security controller
 * 
 * @param status New security status
 * @param save Save status to DB
 * 
 * @return true/false as result of status switching
 */
bool SecurityStatusSet(bool status, bool save);

/**
 * @brief Switch alarm status for security controller
 * 
 * @param status New security alarm status
 * @param save Save status to DB
 * 
 * @return true/false as result of status switching
 */
bool SecurityAlarmSet(bool status, bool save);

/**
 * @brief Get security Alarm status
 */
bool SecurityAlarmGet();

/**
 * @brief Get current security status from security controller
 * 
 * @return Current security status for current controller
 */
bool SecurityStatusGet();

/**
 * @brief Add new iButton key for controller
 * 
 * @param key New security iButton key
 */
void SecurityKeyAdd(const SecurityKey *key);

/**
 * @brief Add new security sensor for controller
 * 
 * @param sensor New security sensor 
 */
void SecuritySensorAdd(const SecuritySensor *sensor);

/**
 * @brief Get security sensor from controller by name
 * 
 * @param name Security sensor name
 * 
 * @return Found security sensor or NULL if not found
 */
SecuritySensor *SecuritySensorGet(const const char *name);

/**
 * @brief Get all security controller's sensors
 * 
 * @return List of sensors for current security controller
 */
GList **SecuritySensorsGet();

#endif /* __SECURITY_CTRL_H__ */
