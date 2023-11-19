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
#include <utils/configs/configs.h>
#include <utils/configs/cfgsecurity.h>
#include <utils/configs/cfgmeteo.h>
#include <utils/configs/cfgsocket.h>
#include <core/gpio.h>
#include <core/extenders.h>
#include <core/lcd.h>
#include <net/notifier.h>
#include <net/web/webserver.h>
#include <net/tgbot/tgbot.h>
#include <net/tgbot/tgmenu.h>
#include <db/database.h>
#include <stack/stack.h>

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
    size_t          ext_index;
    json_t          *ext_value;
    char            err[ERROR_STR_LEN];
    Database        db;
    GpioPin         *gpio = NULL;

    snprintf(full_path, STR_LEN, "%s%s", path, CONFIGS_CONTROLLERS_FILE);

    json_t *data = json_load_file(full_path, 0, &error);
    if (!data) {
        return false;
    }

    if (!CfgSecurityLoad(data)) {
        json_decref(data);
        Log(LOG_TYPE_INFO, "CONFIGS", "Failed to load security configs");
        return false;
    }

    if (!CfgMeteoLoad(data)) {
        json_decref(data);
        Log(LOG_TYPE_INFO, "CONFIGS", "Failed to load meteo configs");
        return false;
    }

    if (!CfgSocketLoad(data)) {
        json_decref(data);
        Log(LOG_TYPE_INFO, "CONFIGS", "Failed to load socket configs");
        return false;
    }

    json_decref(data);
    return true;
}

static bool PlcRead(const char *path)
{
    char            full_path[STR_LEN];
    json_error_t    error;
    char            err[ERROR_STR_LEN];
    size_t          index;
    json_t          *value;

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

    json_t *tgbot = json_object_get(data, "tgbot");
    TgBotTokenSet(json_string_value(json_object_get(tgbot, "token")));
    if (json_boolean_value(json_object_get(tgbot, "enabled"))) {
        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Telegram bot token: \"%s\"", json_string_value(json_object_get(tgbot, "token")));
        json_array_foreach(json_object_get(tgbot, "users"), index, value) {
            TgBotUser *user = (TgBotUser *)malloc(sizeof(TgBotUser));
            TgMenu *menu = (TgMenu *)malloc(sizeof(TgMenu));

            strncpy(user->name, json_string_value(json_object_get(value, "name")), STR_LEN);
            user->chat_id = json_integer_value(json_object_get(value, "id"));
            menu->from = user->chat_id;
            menu->level = TG_MENU_LVL_MAIN;

            TgBotUserAdd(user);
            TgMenuAdd(menu);
            LogF(LOG_TYPE_INFO, "CONFIGS", "Add Telegram bot user: \"%s\"", user->name);
        }
    } else {
        TgBotDisable();
        Log(LOG_TYPE_INFO, "CONFIGS", "Telegram bot disabled");
    }

    /**
     * Stack configs
     */

    json_t *jstack = json_object_get(data, "stack");
    json_array_foreach(json_object_get(jstack, "units"), index, value) {
        StackUnit *unit = StackUnitNew(
            json_integer_value(json_object_get(value, "id")),
            json_string_value(json_object_get(value, "name")),
            json_string_value(json_object_get(value, "ip")),
            json_integer_value(json_object_get(value, "port"))
        );
        StackUnitAdd(unit);
        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Stack unit: \"%s\"", unit->name);
    }

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
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load Factory configs");
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
