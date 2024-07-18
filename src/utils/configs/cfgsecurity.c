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
    if (jsecurity == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security not found");
        return false;
    }

    json_t *jgpio = json_object_get(jsecurity, "gpio");
    if (jgpio == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security GPIO not found");
        return false;
    }

    json_t *jstatus = json_object_get(jgpio, "status");
    if (jstatus == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security GPIO status not found");
        return false;
    }

    GpioPin *status = GpioPinGet(json_string_value(jstatus));
    if (status == NULL) {
        LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller error: Status LED GPIO \"%s\" not found", json_string_value(jstatus));
        return false;
    }

    json_t *jrelay = json_object_get(jgpio, "relay");
    if (jrelay == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security GPIO status not found");
        return false;
    }

    GpioPin *relay = GpioPinGet(json_string_value(jrelay));
    if (relay == NULL) {
        LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller error: Alarm Relay GPIO \"%s\" not found",
            json_string_value(jrelay));
        return false;
    }

    SecurityGpioSet(SECURITY_GPIO_STATUS_LED, status);
    SecurityGpioSet(SECURITY_GPIO_ALARM_RELAY, relay);

    return true;
}

static bool CfgSecuritySensorsLoad(json_t *jsecurity)
{
    size_t  ext_index;
    json_t  *ext_value;

    if (jsecurity == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security not found");
        return false;
    }

    json_t *jsensors = json_object_get(jsecurity, "sensors");
    if (jsensors == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security sensors not found");
        return false;
    }

    json_array_foreach(jsensors, ext_index, ext_value) {
        SecuritySensorType  type;
        GpioPin             *gpio = NULL;

        json_t *jtype = json_object_get(ext_value, "type");
        if (jtype == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Security sensors type not found");
            return false;
        }

        const char *type_str = json_string_value(jtype);
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

        json_t *jgpio = json_object_get(ext_value, "gpio");
        if (jgpio == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Security sensors GPIO not found");
            return false;
        }

        gpio = GpioPinGet(json_string_value(jgpio));
        if (gpio == NULL) {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Security sensor error: GPIO \"%s\" not found in list",
                json_string_value(jgpio));
            return false;
        }

        json_t *jname = json_object_get(ext_value, "name");
        if (jname == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Security sensors name not found");
            return false;
        }

        json_t *jtg = json_object_get(ext_value, "telegram");
        if (jtg == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Security sensors telegram not found");
            return false;
        }

        json_t *jsms = json_object_get(ext_value, "sms");
        if (jsms == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Security sensors sms not found");
            return false;
        }

        json_t *jalarm = json_object_get(ext_value, "alarm");
        if (jalarm == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Security sensors alarm not found");
            return false;
        }

        SecuritySensor *sensor = SecuritySensorNew(
            json_string_value(jname),
            type,
            gpio,
            json_boolean_value(jtg),
            json_boolean_value(jsms),
            json_boolean_value(jalarm)
        );

        SecuritySensorAdd(sensor);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Security sensor name: \"%s\" gpio: \"%s\" type: \"%s\" telegram: \"%d\" sms: \"%d\" alarm: \"%d\"",
            sensor->name, json_string_value(jgpio),
            type_str, sensor->telegram, sensor->sms, sensor->alarm);
    }

    return true;
}

static bool CfgSecurityKeysLoad(json_t *jsecurity)
{
    size_t  ext_index;
    json_t  *ext_value;

    if (jsecurity == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security not found");
        return false;
    }

    json_t *jkeys = json_object_get(jsecurity, "keys");
    if (jkeys == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security keys not found");
        return false;
    }

    json_array_foreach(jkeys, ext_index, ext_value) {
        json_t *jname = json_object_get(ext_value, "name");
        if (jname == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Security keys name not found");
            return false;
        }

        json_t *jid = json_object_get(ext_value, "id");
        if (jid == NULL) {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Security keys id not found");
            return false;
        }

        SecurityKey *key = SecurityKeyNew(
            json_string_value(jname),
            json_string_value(jid)
        );

        SecurityKeyAdd(key);

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add security key: \"%s\"", key->name);
    }
    return true;
}

static bool CfgSecuritySoundLoad(json_t *jsecurity)
{
    json_t *jsound = json_object_get(jsecurity, "sound");
    if (jsound == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security sound not found");
        return false;
    }

    json_t *jenter = json_object_get(jsound, "enter");
    if (jenter == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security sound enter not found");
        return false;
    }

    json_t *jexit = json_object_get(jsound, "exit");
    if (jexit == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security sound exit not found");
        return false;
    }

    json_t *jalarm = json_object_get(jsound, "alarm");
    if (jalarm == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security sound alarm not found");
        return false;
    }

    SecuritySoundSet(SECURITY_SOUND_ENTER, json_boolean_value(jenter));
    SecuritySoundSet(SECURITY_SOUND_EXIT, json_boolean_value(jexit));
    SecuritySoundSet(SECURITY_SOUND_ALARM, json_boolean_value(jalarm));

    return true;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool CfgSecurityLoad(json_t *data)
{
    if (data == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Security data not found");
        return false;
    }

    Log(LOG_TYPE_INFO, "CONFIGS", "Add Security controller");

    json_t *jsecurity = json_object_get(data, "security");
    if (jsecurity == NULL) {
        Log(LOG_TYPE_WARN, "CONFIGS", "Security data not found");
        return true;
    }

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
