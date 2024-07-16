/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023-2024 Denisov Smart Devices Limited             */
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
#include <utils/configs/cfgtank.h>
#include <utils/configs/cfgwaterer.h>
#include <core/gpio.h>
#include <core/extenders.h>
#include <core/lcd.h>
#include <net/notifier.h>
#include <net/web/webserver.h>
#include <net/tgbot/tgbot.h>
#include <net/tgbot/tgmenu.h>
#include <db/database.h>
#include <stack/stack.h>
#include <scenario/scenario.h>
#include <cam/camera.h>
#include <plc/plc.h>
#include <plc/menu.h>
#include <controllers/meteo.h>
#include <controllers/socket.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool FactoryRead(const char *path, ConfigsFactory *factory)
{
    json_error_t    error;
    char            full_path[STR_LEN];
    json_t          *jval;

    if (path == NULL || factory == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Factory or path is empty");
        return false;
    }

    strncpy(full_path, path, STR_LEN);
    strcat(full_path, CONFIGS_FACTORY_FILE);

    json_t *data = json_load_file(full_path, 0, &error);
    if (data == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load factory data");
        return false;
    }

    jval = json_object_get(data, "board");
    if (jval == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "Board not found");
        return false;
    }
    strncpy(factory->board, json_string_value(jval), SHORT_STR_LEN);

    jval = json_object_get(data, "revision");
    if (jval == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "Revision not found");
        return false;
    }
    strncpy(factory->revision, json_string_value(jval), SHORT_STR_LEN);

    json_decref(data);
    return true;
}

static bool BoardRead(const char *path, const ConfigsFactory *factory)
{
    char            full_path[STR_LEN];
    json_error_t    error;
    size_t          index;
    json_t          *value, *jval;
    char            err[ERROR_STR_LEN];

    if (path == NULL || factory == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Factory or path is empty");
        return false;
    }

    strncpy(full_path, path, STR_LEN);
    strcat(full_path, "boards/");
    strcat(full_path, factory->board);
    strcat(full_path, "-");
    strcat(full_path, factory->revision);
    strcat(full_path, ".json");

    json_t *data = json_load_file(full_path, 0, &error);
    if (data == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Board data not found");
        return false;
    }

    /**
     * Reading Extenders configs
     */

    jval = json_object_get(data, "extenders");
    if (jval == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "Extenders not found");
        return false;
    }

    json_array_foreach(jval, index, value) {
        ExtenderType type;

        json_t *jtype = json_object_get(value, "type");
        if (jtype == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "Extender type not found");
            return false;
        }

        const char *type_str = json_string_value(jtype);
        if (!strcmp(type_str, "pcf8574")) {
            type = EXT_TYPE_PCF_8574;
        } else if (!strcmp(type_str, "mcp23017")) {
            type = EXT_TYPE_MCP_23017;
        } else if (!strcmp(type_str, "ads1115")) {
            type = EXT_TYPE_ADS_1115;
        } else {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown Extender type \"%s\"", type_str);
            return false;
        }

        json_t *jname = json_object_get(value, "name");
        if (jname == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "Extender name not found");
            return false;
        }
        json_t *jbus = json_object_get(value, "bus");
        if (jbus == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "Extender bus not found");
            return false;
        }
        json_t *jaddr = json_object_get(value, "addr");
        if (jaddr == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "Extender addr not found");
            return false;
        }
        json_t *jbase = json_object_get(value, "base");
        if (jbase == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "Extender base not found");
            return false;
        }

        Extender *ext = ExtenderNew(
            json_string_value(jname),
            type,
            json_integer_value(jbus),
            json_integer_value(jaddr),
            json_integer_value(jbase)
        );

        if (!ExtenderAdd(ext, err)) {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Failed to add Extender \"%s\": %s", ext->name, err);
            return false;
        }

        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Extender name: \"%s\" type: \"%s\" bus: \"%u\" addr: \"%u\" base: \"%u\"",
                ext->name, type_str, ext->bus, ext->addr, ext->base);
    }

    /**
     * Reading GPIO configs
     */

    json_array_foreach(json_object_get(data, "gpio"), index, value) {
        GpioMode    mode;
        GpioPull    pull;
        GpioType    type;
        json_t      *jval;

        jval = json_object_get(value, "type");
        if (jval == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "GPIO type not found");
            return false;
        }

        const char *type_str = json_string_value(jval);
        if (!strcmp(type_str, "analog")) {
            type = GPIO_TYPE_ANALOG;
        } else if (!strcmp(type_str, "digital")) {
            type = GPIO_TYPE_DIGITAL;
        } else {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown GPIO pin type \"%s\"", type_str);
            return false;
        }

        jval = json_object_get(value, "mode");
        if (jval == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "GPIO mode not found");
            return false;
        }

        const char *mode_str = json_string_value(jval);
        if (!strcmp(mode_str, "input")) {
            mode = GPIO_MODE_INPUT;
        } else if (!strcmp(mode_str, "output")) {
            mode = GPIO_MODE_OUTPUT;
        } else {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown GPIO pin mode \"%s\"", mode_str);
            return false;
        }

        jval = json_object_get(value, "pull");
        if (jval == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "GPIO pull not found");
            return false;
        }

        const char *pull_str = json_string_value(jval);
        if (!strcmp(pull_str, "up")) {
            pull = GPIO_PULL_UP;
        } else if (!strcmp(pull_str, "down")) {
            pull = GPIO_PULL_DOWN;
        } else if (!strcmp(pull_str, "none")) {
            pull = GPIO_PULL_NONE;
        } else {
            json_decref(data);
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown GPIO pin pull \"%s\"", pull_str);
            return false;
        }

        json_t *jname = json_object_get(value, "name");
        if (jname == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "GPIO name not found");
            return false;
        }

        json_t *jpin = json_object_get(value, "pin");
        if (jname == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "GPIO pin not found");
            return false;
        }

        GpioPin *pin = GpioPinNew(
                json_string_value(jname),
                type,
                json_integer_value(jpin),
                mode,
                pull
            );

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
        json_t *jname = json_object_get(value, "name");
        if (jname == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "LCD name not found");
            return false;
        }

        json_t *jrs = json_object_get(value, "rs");
        if (jrs == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "LCD rs not found");
            return false;
        }

        json_t *jrw = json_object_get(value, "rw");
        if (jrw == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "LCD rw not found");
            return false;
        }

        json_t *je = json_object_get(value, "e");
        if (je == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "LCD e not found");
            return false;
        }

        json_t *jk = json_object_get(value, "k");
        if (jk == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "LCD k not found");
            return false;
        }

        json_t *jd4 = json_object_get(value, "d4");
        if (jd4 == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "LCD d4 not found");
            return false;
        }

        json_t *jd5 = json_object_get(value, "d5");
        if (jd5 == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "LCD d5 not found");
            return false;
        }

        json_t *jd6 = json_object_get(value, "d6");
        if (jd6 == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "LCD d6 not found");
            return false;
        }

        json_t *jd7 = json_object_get(value, "d7");
        if (jd7 == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "LCD d7 not found");
            return false;
        }

        LCD *lcd = LcdNew(
                json_string_value(jname),
                json_integer_value(jrs),
                json_integer_value(jrw),
                json_integer_value(je),
                json_integer_value(jk),
                json_integer_value(jd4),
                json_integer_value(jd5),
                json_integer_value(jd6),
                json_integer_value(jd7)
            );

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

    if (path == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Controllers path is empty");
        return false;
    }

    snprintf(full_path, STR_LEN, "%s%s", path, CONFIGS_CONTROLLERS_FILE);

    json_t *data = json_load_file(full_path, 0, &error);
    if (!data) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Controllers data not found");
        return false;
    }

    if (!CfgSecurityLoad(data)) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load security configs");
        return false;
    }

    if (!CfgMeteoLoad(data)) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load meteo configs");
        return false;
    }

    if (!CfgSocketLoad(data)) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load socket configs");
        return false;
    }

    if (!CfgTankLoad(data)) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load tank configs");
        return false;
    }

    if (!CfgWatererLoad(data)) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load waterer configs");
        return false;
    }

    json_decref(data);
    return true;
}

static bool PlcRead(const char *path)
{
    char            full_path[STR_LEN];
    json_error_t    error;
    size_t          index, ext_index;
    json_t          *value, *ext_value;
    GpioPin         *gpio = NULL;

    if (path == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC path is empty");
        return false;
    }

    snprintf(full_path, STR_LEN, "%s%s", path, CONFIGS_PLC_FILE);

    json_t *data = json_load_file(full_path, 0, &error);
    if (data == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC data not found");
        return false;
    }

    json_t *jglobal = json_object_get(data, "global");
    if (jglobal == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC global configs not found");
        return false;
    }
    json_t *jgpio = json_object_get(jglobal, "gpio");
    if (jgpio == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC GPIO configs not found");
        return false;
    }

    json_t *jalarm = json_object_get(jgpio, "alarm");
    if (jalarm == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC GPIO alarm not found");
        return false;
    }

    gpio = GpioPinGet(json_string_value(jalarm));
    if (gpio == NULL) {
        LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller error: Alarm LED GPIO \"%s\" not found",
            json_string_value(jalarm));
        return false;
    }
    PlcGpioSet(PLC_GPIO_ALARM_LED, gpio);

    json_t *jbuzzer = json_object_get(jgpio, "buzzer");
    if (jbuzzer == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC GPIO buzzer not found");
        return false;
    }

    gpio = GpioPinGet(json_string_value(jbuzzer));
    if (gpio == NULL) {
        LogF(LOG_TYPE_ERROR, "CONFIGS", "Security controller error: Buzzer GPIO \"%s\" not found",
            json_string_value(jbuzzer));
        return false;
    }
    PlcGpioSet(PLC_GPIO_BUZZER, gpio);

    json_t *jserver = json_object_get(data, "server");
    if (jserver == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC server not found");
        return false;
    }

    json_t *jip = json_object_get(jserver, "ip");
    if (jip == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC server IP not found");
        return false;
    }

    json_t *jport = json_object_get(jserver, "port");
    if (jport == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC server port not found");
        return false;
    }

    const char *ip = json_string_value(jip);
    const unsigned port = json_integer_value(jport);
    WebServerCredsSet(ip, port);
    LogF(LOG_TYPE_INFO, "CONFIGS", "Add Web Server at ip: \"%s\" port: \"%u\"", ip, port);

    json_t *notifier = json_object_get(data, "notifier");
    if (notifier == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC notifier not found");
        return false;
    }

    json_t *jtg = json_object_get(notifier, "telegram");
    if (jtg == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC telegram not found");
        return false;
    }

    json_t *jbot = json_object_get(jtg, "bot");
    if (jbot == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC telegram bot not found");
        return false;
    }

    json_t *jchat = json_object_get(jtg, "chat");
    if (jchat == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC telegram chat not found");
        return false;
    }

    const char *bot = json_string_value(jbot);
    const unsigned chat = json_integer_value(jchat);
    NotifierTelegramCredsSet(bot, chat);
    LogF(LOG_TYPE_INFO, "CONFIGS", "Add Telegram bot Notifier token: \"%s\" chat: \"%u\"", bot, chat);

    json_t *jsms = json_object_get(notifier, "sms");
    if (jsms == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC SMS not found");
        return false;
    }

    json_t *japi = json_object_get(jsms, "api");
    if (japi == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC SMS API not found");
        return false;
    }

    json_t *jphone = json_object_get(jsms, "phone");
    if (jphone == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC SMS phone not found");
        return false;
    }

    const char *api = json_string_value(japi);
    const char *phone = json_string_value(jphone);
    NotifierSmsCredsSet(api, phone);
    LogF(LOG_TYPE_INFO, "CONFIGS", "Add SMS Notifier token: \"%s\" phone: \"%s\"", api, phone);

    json_t *jtgbot = json_object_get(data, "tgbot");
    if (jtgbot == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC tgbot not found");
        return false;
    }

    json_t *jtoken = json_object_get(jtgbot, "token");
    if (jtoken == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC tgbot token not found");
        return false;
    }

    json_t *jen = json_object_get(jtgbot, "enabled");
    if (jtoken == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC tgbot enabled not found");
        return false;
    }

    TgBotTokenSet(json_string_value(jtoken));
    if (json_boolean_value(jen)) {
        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Telegram bot token: \"%s\"", json_string_value(jtoken));

        json_t *jusers = json_object_get(jtgbot, "users");
        if (jusers == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "PLC tgbot users not found");
            return false;
        }

        json_array_foreach(jusers, index, value) {
            json_t *jname = json_object_get(value, "name");
            if (jname == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "PLC tgbot user name not found");
                return false;
            }

            json_t *jid = json_object_get(value, "id");
            if (jid == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "PLC tgbot user id not found");
                return false;
            }

            TgBotUser *user = TgBotUserNew(
                json_string_value(jname),
                json_integer_value(jid)
            );

            TgBotUserAdd(user);

            TgMenu *menu = TgMenuNew(
                user->chat_id
            );
            
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
    if (jstack == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC stack not found");
        return false;
    }

    json_array_foreach(jstack, index, value) {
        json_t *jid = json_object_get(value, "id");
        if (jid == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "PLC stack id not found");
            return false;
        }

        json_t *jname = json_object_get(value, "name");
        if (jname == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "PLC stack name not found");
            return false;
        }

        json_t *jip = json_object_get(value, "ip");
        if (jip == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "PLC stack ip not found");
            return false;
        }

        json_t *jport = json_object_get(value, "port");
        if (jport == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "PLC stack port not found");
            return false;
        }

        StackUnit *unit = StackUnitNew(
            json_integer_value(jid),
            json_string_value(jname),
            json_string_value(jip),
            json_integer_value(jport)
        );
        StackUnitAdd(unit);
        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Stack unit: \"%s\"", unit->name);
    }

    /**
     * Cameras configs
     */

    json_t *jcam = json_object_get(data, "cam");
    if (jcam == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC cam not found");
        return false;
    }

    json_array_foreach(jcam, index, value) {
        CameraType type;

        json_t *jcamt = json_object_get(value, "type");
        if (jcamt == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "PLC cam type not found");
            return false;
        }

        const char *type_str = json_string_value(jcamt);

        if (!strcmp(type_str, "ipcam")) {
            type = CAM_TYPE_IP;
        } else {
            LogF(LOG_TYPE_ERROR, "CONFIGS", "Invalid camera type \"%s\"", type_str);
            return false;
        }

        json_t *jcamn = json_object_get(value, "name");
        if (jcamn == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "PLC cam name not found");
            return false;
        }

        Camera *cam = CameraNew(
            json_string_value(jcamn),
            type
        );

        if (type == CAM_TYPE_IP) {
            json_t *jipcam = json_object_get(value, "ipcam");
            if (jipcam == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "PLC cam ipcam not found");
                return false;
            }

            json_t *jip = json_object_get(jipcam, "ip");
            if (jip == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "PLC cam ipcam not found");
                return false;
            }
            strncpy(cam->ipcam.ip, json_string_value(jip), SHORT_STR_LEN);

            json_t *jlogin = json_object_get(jipcam, "login");
            if (jlogin == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "PLC cam ipcam login not found");
                return false;
            }
            strncpy(cam->ipcam.login, json_string_value(jlogin), SHORT_STR_LEN);

            json_t *jpass = json_object_get(jipcam, "password");
            if (jpass == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "PLC cam ipcam password not found");
                return false;
            }
            strncpy(cam->ipcam.password, json_string_value(jpass), SHORT_STR_LEN);

            json_t *jstream = json_object_get(jipcam, "stream");
            if (jstream == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "PLC cam ipcam stream not found");
                return false;
            }
            cam->ipcam.stream = json_integer_value(jstream);
        }

        CameraAdd(cam);
        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Camera: \"%s\"", cam->name);
    }

    /**
     * Menu configs
     */

    json_t *jmenu = json_object_get(data, "menu");
    if (jmenu == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu not found");
        return false;
    }

    json_t *jlcd = json_object_get(jmenu, "lcd");
    if (jlcd == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu LCD not found");
        return false;
    }

    if (strcmp(json_string_value(jlcd), "none")) {
        LCD *lcd = LcdGet(json_string_value(jlcd));
        if (lcd == NULL) {
            json_decref(data);
        	LogF(LOG_TYPE_ERROR, "CONFIGS", "LCD \"%s\" of menu not found", json_string_value(jlcd));
        	return false;
        }
        MenuLcdSet(lcd);
        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Menu LCD \"%s\"", json_string_value(jlcd));
    }

    Log(LOG_TYPE_INFO, "CONFIGS", "Add Menu GPIOs");

    jgpio = json_object_get(jmenu, "gpio");
    if (jgpio == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu GPIO not found");
        return false;
    }

    json_t *jup = json_object_get(jgpio, "up");
    if (jup == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu GPIO up not found");
        return false;
    }

    gpio = GpioPinGet(json_string_value(jup));
    if (gpio == NULL) {
        LogF(LOG_TYPE_ERROR, "CONFIGS", "Menu button error: GPIO \"%s\" not found",
            json_string_value(jup));
        return false;
    }
    MenuGpioSet(MENU_GPIO_UP, gpio);

    json_t *jmid = json_object_get(jgpio, "middle");
    if (jmid == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu GPIO middle not found");
        return false;
    }

    gpio = GpioPinGet(json_string_value(jmid));
    if (gpio == NULL) {
        LogF(LOG_TYPE_ERROR, "CONFIGS", "Menu button error: GPIO \"%s\" not found",
            json_string_value(jmid));
        return false;
    }
    MenuGpioSet(MENU_GPIO_MIDDLE, gpio);

    json_t *jdn = json_object_get(jgpio, "down");
    if (jdn == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu GPIO down not found");
        return false;
    }

    gpio = GpioPinGet(json_string_value(jdn));
    if (gpio == NULL) {
        LogF(LOG_TYPE_ERROR, "CONFIGS", "Menu button error: GPIO \"%s\" not found",
            json_string_value(jdn));
        return false;
    }
    MenuGpioSet(MENU_GPIO_DOWN, gpio);

    json_t *jlevels = json_object_get(jmenu, "levels");
    if (jlevels == NULL) {
        json_decref(data);
        Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels not found");
        return false;
    }

    json_array_foreach(jlevels, index, value) {
        json_t *jname = json_object_get(value, "name");
        if (jname == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels name not found");
            return false;
        }

        MenuLevel *level = MenuLevelNew(json_string_value(jname));
        LogF(LOG_TYPE_INFO, "CONFIGS", "Add Menu Level \"%s\"", level->name);

        json_t *jvalues = json_object_get(value, "values");
        if (jvalues == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels values not found");
            return false;
        }

        json_array_foreach(jvalues, ext_index, ext_value) {
            MenuController ctrl;

            json_t *jctrl = json_object_get(ext_value, "ctrl");
            if (jctrl == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels ctrl not found");
                return false;
            }
            const char *sctrl = json_string_value(jctrl);

            if (!strcmp(sctrl, "meteo")) {
                ctrl = MENU_CTRL_METEO;
            } else if (!strcmp(sctrl, "time")) {
                ctrl = MENU_CTRL_TIME;
            } else if (!strcmp(sctrl, "tank")) {
                ctrl = MENU_CTRL_TANK;
            } else if (!strcmp(sctrl, "socket")) {
                ctrl = MENU_CTRL_SOCKET;
            } else if (!strcmp(sctrl, "light")) {
                ctrl = MENU_CTRL_LIGHT;
            } else if (!strcmp(sctrl, "security")) {
                ctrl = MENU_CTRL_SECURITY;
            } else {
                LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown ctrl type");
                return false;
            }

            json_t *jalias = json_object_get(ext_value, "alias");
            if (jalias == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels alias not found");
                return false;
            }

            json_t *jrow = json_object_get(ext_value, "row");
            if (jrow == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels row not found");
                return false;
            }

            json_t *jcol = json_object_get(ext_value, "col");
            if (jcol == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels alias not found");
                return false;
            }

            LogF(LOG_TYPE_INFO, "CONFIGS", "Add Menu value ctrl: \"%s\" alias \"%s\"",
                sctrl,
                json_string_value(jalias)
            );

            MenuValue *value = MenuValueNew(
                json_integer_value(jrow),
                json_integer_value(jcol),
                json_string_value(jalias),
                ctrl
            );

            if (ctrl == MENU_CTRL_METEO) {
                json_t *jmeteo = json_object_get(ext_value, "meteo");
                if (jmeteo == NULL) {
                    json_decref(data);
                    Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels meteo not found");
                    return false;
                }

                MeteoSensor *sensor = MeteoSensorGet(
                    json_string_value(jmeteo)
                );

                if (sensor == NULL) {
                    LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown menu meteo sensor");
                    return false;
                }

                value->meteo.sensor = sensor;
            } else if (ctrl == MENU_CTRL_TANK) {
                json_t *jtank = json_object_get(ext_value, "tank");
                if (jtank == NULL) {
                    json_decref(data);
                    Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels tank not found");
                    return false;
                }

                json_t *jname = json_object_get(jtank, "name");
                if (jname == NULL) {
                    json_decref(data);
                    Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels tank name not found");
                    return false;
                }

                Tank *tank = TankGet(
                    json_string_value(jname)
                );

                if (tank == NULL) {
                    LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown menu tank");
                    return false;
                }

                value->tank.tank = tank;

                json_t *jparam = json_object_get(jtank, "param");
                if (jparam == NULL) {
                    json_decref(data);
                    Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels param not found");
                    return false;
                }
                const char *sparam = json_string_value(jparam);

                if (!strcmp(sparam, "pump")) {
                    value->tank.param = MENU_TANK_PUMP;
                } else if (!strcmp(sparam, "valve")) {
                    value->tank.param = MENU_TANK_VALVE;
                } else if (!strcmp(sparam, "level")) {
                    value->tank.param = MENU_TANK_LEVEL;
                }
            } else if (ctrl == MENU_CTRL_SOCKET) {
                json_t *jsocket = json_object_get(ext_value, "socket");
                if (jsocket == NULL) {
                    json_decref(data);
                    Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels socket not found");
                    return false;
                }

                Socket *socket = SocketGet(
                    json_string_value(jsocket)
                );

                if (socket == NULL) {
                    LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown menu socket");
                    return false;
                }

                value->socket.sock = socket;
            } else if (ctrl == MENU_CTRL_LIGHT) {
                json_t *jlight = json_object_get(ext_value, "light");
                if (jlight == NULL) {
                    json_decref(data);
                    Log(LOG_TYPE_ERROR, "CONFIGS", "PLC menu levels light not found");
                    return false;
                }

                Socket *socket = SocketGet(
                    json_string_value(jlight)
                );

                if (socket == NULL) {
                    LogF(LOG_TYPE_ERROR, "CONFIGS", "Unknown menu light");
                    return false;
                }

                value->light.sock = socket;
            }

            MenuValueAdd(level, value);
        }

        MenuLevelAdd(level);
    }

    json_decref(data);
    return true;
}

static bool ScenarioRead(const char *path)
{
    char            full_path[STR_LEN];
    json_error_t    error;
    size_t          index;
    json_t          *value;

    if (path == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Scenario path is empty");
        return false;
    }

    snprintf(full_path, STR_LEN, "%s%s", path, CONFIGS_SCENARIO_FILE);

    json_t *data = json_load_file(full_path, 0, &error);
    if (!data) {
        return false;
    }

    json_array_foreach(json_object_get(data, "scenario"), index, value) {
        Scenario *scenario = (Scenario *)malloc(sizeof(Scenario));

        json_t *junit = json_object_get(value, "unit");
        if (junit == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "Scenario unit not found");
            return false;
        }

        scenario->unit = json_integer_value(junit);

        json_t *jtype = json_object_get(value, "type");
        if (jtype == NULL) {
            json_decref(data);
            Log(LOG_TYPE_ERROR, "CONFIGS", "Scenario type not found");
            return false;
        }
        const char *stype = json_string_value(jtype);

        if (!strcmp(stype, "inhome")) {
            scenario->type = SCENARIO_IN_HOME;
        } else if (!strcmp(stype, "outhome")) {
            scenario->type = SCENARIO_OUT_HOME;
        } else {
            Log(LOG_TYPE_ERROR, "CONFIGS", "Invalid scenario type");
            return false;
        }

        if (!strcmp(json_string_value(json_object_get(value, "ctrl")), "socket")) {
            json_t *jsocket = json_object_get(value, "socket");
            if (jsocket == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "Scenario socket not found");
                return false;
            }

            json_t *jname = json_object_get(jsocket, "name");
            if (jname == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "Scenario socket name not found");
                return false;
            }

            json_t *jstatus = json_object_get(jsocket, "status");
            if (jstatus == NULL) {
                json_decref(data);
                Log(LOG_TYPE_ERROR, "CONFIGS", "Scenario socket status not found");
                return false;
            }
    
            strncpy(scenario->socket.name, json_string_value(jname), SHORT_STR_LEN);
            scenario->socket.status = json_boolean_value(jstatus);
            scenario->ctrl = SECURITY_CTRL_SOCKET;
        }

        ScenarioAdd(scenario);
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

    if (path == NULL) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Configs path is empty");
        return false;
    }

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

    if (!ScenarioRead(path)) {
        Log(LOG_TYPE_ERROR, "CONFIGS", "Failed to load Scenario configs");
        return false;
    }

    return true;
}
