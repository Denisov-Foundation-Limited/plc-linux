/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <utils/log.h>
#include <controllers/security.h>
#include <core/gpio.h>
#include <utils/configs/cfgsecurity.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool CfgSecurityGpioLoad(json_t *jsecurity)
{
    GpioPin *gpio = NULL;

    json_t *jgpio = json_object_get(jsecurity, "gpio");
    gpio = GpioPinGet(json_string_value(json_object_get(jgpio, "status")));
    if (gpio == NULL) {
        LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller error: Status LED GPIO \"%s\" not found", json_string_value(json_object_get(jgpio, "status")));
        return false;
    }
    SecurityGpioSet(SECURITY_GPIO_STATUS_LED, gpio);

    gpio = GpioPinGet(json_string_value(json_object_get(jgpio, "buzzer")));
    if (gpio == NULL) {
        LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller error: Buzzer GPIO \"%s\" not found", json_string_value(json_object_get(jgpio, "buzzer")));
        return false;
    }
    SecurityGpioSet(SECURITY_GPIO_BUZZER, gpio);
    
    json_t *jalarm = json_object_get(jgpio, "alarm");
    gpio = GpioPinGet(json_string_value(json_object_get(jalarm, "led")));
    if (gpio == NULL) {
        LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller error: Alarm LED GPIO \"%s\" not found", json_string_value(json_object_get(jalarm, "led")));
        return false;
    }
    SecurityGpioSet(SECURITY_GPIO_ALARM_LED, gpio);

    gpio = GpioPinGet(json_string_value(json_object_get(jalarm, "relay")));
    if (gpio == NULL) {
        LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller error: Alarm Relay GPIO \"%s\" not found",
            json_string_value(json_object_get(jalarm, "relay")));
        return false;
    }
    SecurityGpioSet(SECURITY_GPIO_ALARM_RELAY, gpio);

    return true;
}

static bool CfgSecuritySensorsLoad(json_t *jsecurity)
{
    size_t  ext_index;
    json_t  *ext_value;

    json_array_foreach(json_object_get(jsecurity, "sensors"), ext_index, ext_value) {
        SecuritySensorType  type;
        GpioPin             *gpio = NULL;

        const char *type_str = json_string_value(json_object_get(ext_value, "type"));
        if (!strcmp(type_str, "reed")) {
            type = SECURITY_SENSOR_REED;
        } else if (!strcmp(type_str, "pir")) {
            type = SECURITY_SENSOR_PIR;
        } else if (!strcmp(type_str, "microwave")) {
            type = SECURITY_SENSOR_MICRO_WAVE;
        } else {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown Security sensor type: \"%s\"", type_str);
            return false;
        }

        gpio = GpioPinGet(json_string_value(json_object_get(ext_value, "gpio")));
        if (gpio == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Security sensor error: GPIO \"%s\" not found",
                json_string_value(json_object_get(ext_value, "gpio")));
            return false;
        }

        SecuritySensor *sensor = SecuritySensorNew(
            json_string_value(json_object_get(ext_value, "name")),
            type,
            gpio,
            json_boolean_value(json_object_get(ext_value, "telegram")),
            json_boolean_value(json_object_get(ext_value, "sms")),
            json_boolean_value(json_object_get(ext_value, "alarm"))
        );

        SecuritySensorAdd(sensor);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Security sensor name: \"%s\" gpio: \"%s\" type: \"%s\" telegram: \"%d\" sms: \"%d\" alarm: \"%d\"",
            sensor->name, json_string_value(json_object_get(ext_value, "gpio")),
            type_str, sensor->telegram, sensor->sms, sensor->alarm);
    }

    return true;
}

static bool CfgSecurityKeysLoad(json_t *jsecurity)
{
    size_t  ext_index;
    json_t  *ext_value;

    json_array_foreach(json_object_get(jsecurity, "keys"), ext_index, ext_value) {
        SecurityKey *key = SecurityKeyNew(
            json_string_value(json_object_get(ext_value, "name")),
            json_string_value(json_object_get(ext_value, "id"))
        );

        SecurityKeyAdd(key);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add security key: \"%s\"", key->name);
    }
    return true;
}

static bool CfgSecuritySoundLoad(json_t *jsecurity)
{
    json_t *jsound = json_object_get(jsecurity, "sound");

    SecuritySoundSet(SECURITY_SOUND_ENTER, json_boolean_value(json_object_get(jsound, "enter")));
    SecuritySoundSet(SECURITY_SOUND_EXIT, json_boolean_value(json_object_get(jsound, "exit")));
    SecuritySoundSet(SECURITY_SOUND_ALARM, json_boolean_value(json_object_get(jsound, "alarm")));

    return true;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool CfgSecurityLoad(json_t *data)
{
    json_t *jsecurity = json_object_get(data, "security");
    if (jsecurity == NULL) {
        return false;
    }

    Log(LOG_TYPE_INFO, "CONFIGS", "Add Security controller");

    if (!CfgSecuritySoundLoad(jsecurity)) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load security sound configs");
        return false;
    }

    if (!CfgSecurityGpioLoad(jsecurity)) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load security GPIO configs");
        return false;
    }

    if (!CfgSecuritySensorsLoad(jsecurity)) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load security sensors configs");
        return false;
    }

    if (!CfgSecurityKeysLoad(jsecurity)) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load security keys configs");
        return false;
    }
    
    return true;
}
