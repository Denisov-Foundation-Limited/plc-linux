/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <string.h>
#include <stdlib.h>

#include <jansson.h>

#include <utils/utils.h>
#include <utils/log.h>
#include <utils/configs.h>
#include <core/gpio.h>
#include <core/extenders.h>
#include <core/lcd.h>
#include <net/notifier.h>
#include <net/server.h>
#include <controllers/security.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool FactoryRead(const char *path, ConfigsFactory *factory)
{
    json_error_t    error;
    char            full_path[STR_LEN];

    strncpy(full_path, path, STR_LEN);
    strcat(full_path, CONFIGS_FACTORY_FILE);

    json_t *data = json_load_file(full_path, 0, &error);
    if (!data) {
        return false;
    }

    strncpy(factory->board, json_string_value(json_object_get(data, "board")), SHORT_STR_LEN);
    strncpy(factory->revision, json_string_value(json_object_get(data, "revision")), SHORT_STR_LEN);

    json_decref(data);
    return true;
}

static bool BoardRead(const char *path, const ConfigsFactory *factory)
{
    char            full_path[STR_LEN];
    json_error_t    error;
    size_t          index;
    json_t          *value;
    char            err[ERROR_STR_LEN];

    strncpy(full_path, path, STR_LEN);
    strcat(full_path, "boards/");
    strcat(full_path, factory->board);
    strcat(full_path, "-");
    strcat(full_path, factory->revision);
    strcat(full_path, ".json");

    json_t *data = json_load_file(full_path, 0, &error);
    if (!data) {
        return false;
    }

    /**
     * Reading Extenders configs
     */

    json_array_foreach(json_object_get(data, "extenders"), index, value) {
        Extender *ext = (Extender *)malloc(sizeof(Extender));

        strncpy(ext->name, json_string_value(json_object_get(value, "name")), GPIO_NAME_STR_LEN);
        ext->addr = json_integer_value(json_object_get(value, "addr"));
        ext->base = json_integer_value(json_object_get(value, "base"));
        ext->enabled = json_boolean_value(json_object_get(value, "enabled"));

        const char *type_str = json_string_value(json_object_get(value, "type"));
        if (!strcmp(type_str, "pcf8574")) {
            ext->type = EXT_TYPE_PCF_8574;
        } else if (!strcmp(type_str, "mcp23017")) {
            ext->type = EXT_TYPE_MCP_23017;
        } else if (!strcmp(type_str, "ads1115")) {
            ext->type = EXT_TYPE_ADS_1115;
        } else {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown Extender type \"%s\"", ext->name);
            return false;
        }

        if (!ExtenderAdd(ext, err)) {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Failed to add Extender \"%s\": %s", ext->name, err);
            return false;
        }

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Extender name: \"%s\" type: \"%s\" addr: \"%u\" base: \"%u\" enabled: \"%d\"",
                ext->name, type_str, ext->addr, ext->base, ext->enabled);
    }

    /**
     * Reading GPIO configs
     */

    json_array_foreach(json_object_get(data, "gpio"), index, value) {
        GpioPin *pin = (GpioPin *)malloc(sizeof(GpioPin));

        strncpy(pin->name, json_string_value(json_object_get(value, "name")), SHORT_STR_LEN);
        pin->pin = json_integer_value(json_object_get(value, "pin"));

        const char *type_str = json_string_value(json_object_get(value, "type"));
        if (!strcmp(type_str, "analog")) {
            pin->type = GPIO_TYPE_ANALOG;
        } else if (!strcmp(type_str, "digital")) {
            pin->type = GPIO_TYPE_DIGITAL;
        } else {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown GPIO pin type \"%s\"", pin->name);
            return false;
        }

        const char *mode_str = json_string_value(json_object_get(value, "mode"));
        if (!strcmp(mode_str, "input")) {
            pin->mode = GPIO_MODE_INPUT;
        } else if (!strcmp(mode_str, "output")) {
            pin->mode = GPIO_MODE_OUTPUT;
        } else {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown GPIO pin mode \"%s\"", pin->name);
            return false;
        }

        const char *pull_str = json_string_value(json_object_get(value, "pull"));
        if (!strcmp(pull_str, "up")) {
            pin->pull = GPIO_PULL_UP;
        } else if (!strcmp(pull_str, "down")) {
            pin->pull = GPIO_PULL_DOWN;
        } else if (!strcmp(pull_str, "none")) {
            pin->pull = GPIO_PULL_NONE;
        } else {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown GPIO pin pull \"%s\"", pin->name);
            return false;
        }

        if (!GpioPinAdd(pin, err)) {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Failed to add GPIO pin \"%s\": %s", pin->name, err);
            return false;
        }

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add GPIO name: \"%s\" pin: \"%d\" type: \"%s\" mode: \"%s\" pull: \"%s\"",
                pin->name, pin->pin, type_str, mode_str, pull_str);
    }

    /**
     * Reading LCD configs
     */

    json_array_foreach(json_object_get(data, "lcd"), index, value) {
        LCD *lcd = (LCD *)malloc(sizeof(LCD));

        strncpy(lcd->name, json_string_value(json_object_get(value, "name")), SHORT_STR_LEN);
        lcd->rs = json_integer_value(json_object_get(value, "rs"));
        lcd->rw = json_integer_value(json_object_get(value, "rw"));
        lcd->e = json_integer_value(json_object_get(value, "e"));
        lcd->k = json_integer_value(json_object_get(value, "k"));
        lcd->d4 = json_integer_value(json_object_get(value, "d4"));
        lcd->d5 = json_integer_value(json_object_get(value, "d5"));
        lcd->d6 = json_integer_value(json_object_get(value, "d6"));
        lcd->d7 = json_integer_value(json_object_get(value, "d7"));

        if (!LcdAdd(lcd)) {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Failed to add LCD \"%s\"", lcd->name);
            return false;
        }

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add LCD name: \"%s\"", lcd->name);
    }

    /**
     * Cleanup
     */

    json_decref(data);
    return true;
}

static bool ControllersRead(const char *path)
{
    char            full_path[STR_LEN];
    json_error_t    error;
    size_t          index, ext_index;
    json_t          *value, *ext_value;
    char            err[ERROR_STR_LEN];

    snprintf(full_path, STR_LEN, "%s%s", path, CONFIGS_CONTROLLERS_FILE);

    json_t *data = json_load_file(full_path, 0, &error);
    if (!data) {
        return false;
    }

    /**
     * Reading Extenders configs
     */

    json_array_foreach(json_object_get(data, "security"), index, value) {
        SecurityController *ctrl = (SecurityController *)malloc(sizeof(SecurityController));

        ctrl->last_alarm = false;
        ctrl->alarm = false;
        ctrl->status = false;
        ctrl->sensors = NULL;
        ctrl->keys = NULL;

        strncpy(ctrl->name, json_string_value(json_object_get(value, "name")), SHORT_STR_LEN);
        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Security controller: \"%s\"", ctrl->name);

        /**
         * Add security GPIOs
         */

        json_t *jgpio = json_object_get(value, "gpio");
        ctrl->gpio[SECURITY_GPIO_STATUS_LED] = GpioPinGet(json_string_value(json_object_get(jgpio, "status")));
        if (ctrl->gpio[SECURITY_GPIO_STATUS_LED] == NULL) {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller \"%s\" error: Status LED GPIO \"%s\" not found", ctrl->name, json_string_value(json_object_get(jgpio, "status")));
            return false;
        }
        ctrl->gpio[SECURITY_GPIO_BUZZER] = GpioPinGet(json_string_value(json_object_get(jgpio, "buzzer")));
        if (ctrl->gpio[SECURITY_GPIO_BUZZER] == NULL) {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller \"%s\" error: Buzzer GPIO \"%s\" not found", ctrl->name, json_string_value(json_object_get(jgpio, "buzzer")));
            return false;
        }
        
        json_t *jalarm = json_object_get(jgpio, "alarm");
        ctrl->gpio[SECURITY_GPIO_ALARM_LED] = GpioPinGet(json_string_value(json_object_get(jalarm, "led")));
        if (ctrl->gpio[SECURITY_GPIO_ALARM_LED] == NULL) {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller \"%s\" error: Alarm LED GPIO \"%s\" not found", ctrl->name, json_string_value(json_object_get(jalarm, "led")));
            return false;
        }
        ctrl->gpio[SECURITY_GPIO_ALARM_RELAY] = GpioPinGet(json_string_value(json_object_get(jalarm, "relay")));
        if (ctrl->gpio[SECURITY_GPIO_ALARM_RELAY] == NULL) {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller \"%s\" error: Alarm Relay GPIO \"%s\" not found",
                ctrl->name, json_string_value(json_object_get(jalarm, "relay")));
            return false;
        }

        /**
         * Add security sensors
         */

        json_array_foreach(json_object_get(value, "sensors"), ext_index, ext_value) {
            SecuritySensor *sensor = (SecuritySensor *)malloc(sizeof(SecuritySensor));

            strncpy(sensor->name, json_string_value(json_object_get(ext_value, "name")), SHORT_STR_LEN);

            sensor->gpio = GpioPinGet(json_string_value(json_object_get(ext_value, "gpio")));
            if (sensor->gpio == NULL) {
                json_decref(data);
                LogF(LOG_TYPE_ERROR, "CONFIGS", "Security sensor \"%s\" error: GPIO \"%s\" not found",
                    sensor->name, json_string_value(json_object_get(ext_value, "gpio")));
                return false;
            }

            const char *type_str = json_string_value(json_object_get(ext_value, "type"));
            if (!strcmp(type_str, "reed")) {
                sensor->type = SECURITY_SENSOR_REED;
            } else if (!strcmp(type_str, "pir")) {
                sensor->type = SECURITY_SENSOR_PIR;
            } else if (!strcmp(type_str, "microwave")) {
                sensor->type = SECURITY_SENSOR_MICRO_WAVE;
            } else {
                json_decref(data);
                LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown Security sensor \"%s\" type: \"%s\"", sensor->name, type_str);
                return false;
            }

            sensor->telegram = json_boolean_value(json_object_get(ext_value, "telegram"));
            sensor->sms = json_boolean_value(json_object_get(ext_value, "sms"));
            sensor->alarm = json_boolean_value(json_object_get(ext_value, "alarm"));
            sensor->detected = false;

            SecuritySensorAdd(ctrl, sensor);

            LogF(LOG_TYPE_INFO, "CONFIGS", "Add Security sensor: \"%s\" gpio: \"%s\" type: \"%s\" telegram: \"%d\" sms: \"%d\" alarm: \"%d\"",
                sensor->name, json_string_value(json_object_get(ext_value, "gpio")),
                type_str, sensor->telegram, sensor->sms, sensor->alarm);
        }

        /**
         * Add security keys
         */

        json_array_foreach(json_object_get(value, "keys"), ext_index, ext_value) {
            SecurityKey *key = (SecurityKey *)malloc(sizeof(SecurityKey));
            strncpy(key->value, json_string_value(ext_value), SHORT_STR_LEN);
            SecurityKeyAdd(ctrl, key);
            LogF(LOG_TYPE_INFO, "CONFIGS", "Add security key: \"%s\"", key->value);
        }

        SecurityControllerAdd(ctrl);
    }

    json_decref(data);
    return true;
}

static bool PlcRead(const char *path)
{
    char            full_path[STR_LEN];
    json_error_t    error;
    char            err[ERROR_STR_LEN];

    snprintf(full_path, STR_LEN, "%s%s", path, CONFIGS_PLC_FILE);

    json_t *data = json_load_file(full_path, 0, &error);
    if (!data) {
        return false;
    }

    json_t *server = json_object_get(data, "server");
    const char *ip = json_string_value(json_object_get(server, "ip"));
    const unsigned port = json_integer_value(json_object_get(server, "port"));
    WebServerCredsSet(ip, port);
    LogF(LOG_TYPE_INFO, "CONFIGS", "Add Web Server at ip: \"%s\" port: \"%u\"", ip, port);

    json_t *notifier = json_object_get(data, "notifier");

    json_t *jtg = json_object_get(notifier, "telegram");
    const char *bot = json_string_value(json_object_get(jtg, "bot"));
    const unsigned chat = json_integer_value(json_object_get(jtg, "chat"));
    NotifierTelegramCredsSet(bot, chat);
    LogF(LOG_TYPE_INFO, "CONFIGS", "Add Telegram bot Notifier token: \"%s\" chat: \"%u\"", bot, chat);

    json_t *jsms = json_object_get(notifier, "sms");
    const char *api = json_string_value(json_object_get(jsms, "api"));
    const char *phone = json_string_value(json_object_get(jsms, "phone"));
    NotifierSmsCredsSet(api, phone);
    LogF(LOG_TYPE_INFO, "CONFIGS", "Add SMS Notifier token: \"%s\" phone: \"%s\"", api, phone);

    json_decref(data);
    return true;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool ConfigsRead(const char *path)
{
    ConfigsFactory factory;

    if (!FactoryRead(path, &factory)) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load ConfigsFactory configs");
        return false;
    }

    if (!BoardRead(path, &factory)) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load Board configs");
        return false;
    }

    if (!ControllersRead(path)) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load Controllers configs");
        return false;
    }

    if (!PlcRead(path)) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load PLC configs");
        return false;
    }

    return true;
}
