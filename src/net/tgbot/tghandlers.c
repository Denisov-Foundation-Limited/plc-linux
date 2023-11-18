/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <stdbool.h>

#include <curl/curl.h>
#include <jansson.h>

#include <utils/utils.h>
#include <utils/log.h>
#include <net/tgbot/tghandlers.h>
#include <net/tgbot/tgmenu.h>
#include <controllers/meteo.h>
#include <stack/stack.h>
#include <stack/rpc.h>
#include <net/web/webclient.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool TgBotResponseSend(const char *token, unsigned id, const char *text, json_t *buttons)
{
    char    url[STR_LEN];
    char    post[EXT_STR_LEN];
    char    buf[BUFFER_LEN_MAX];
    json_t *root = json_object();

    snprintf(url, STR_LEN, "https://api.telegram.org/bot%s/sendMessage", token);

    json_object_set_new(root, "keyboard", buttons);
    char *markup = json_dumps(root, JSON_INDENT(0));
    snprintf(post, EXT_STR_LEN, "chat_id=%d&parse_mode=HTML&text=%s&reply_markup=%s", id, text, markup);
    free(markup);
    json_decref(root);

    return WebClientRequest(WEB_REQ_POST, url, post, buf);
}

static void TgBotButtonAdd(json_t *buttons, const char *name)
{
    json_t *butline = json_array();
    json_array_append_new(butline, json_string(name));
    json_array_append_new(buttons, butline);
}

static void TgBotButtonsAdd(json_t *buttons, unsigned count, const char *name[])
{
    json_t *butline = json_array();

    for (unsigned i = 0; i < count; i++) {
        json_array_append_new(butline, json_string(name[i]));
    }

    json_array_append_new(buttons, butline);
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void StackSelectMenuProcess(const char *token, unsigned from, const char *message)
{
    json_t  *buttons = json_array();
    GList   *units = NULL;

    StackActiveUnitsGet(&units);

    for (GList *u = units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;
        TgBotButtonAdd(buttons, unit->name);
    }
    g_list_free(units);

    TgBotButtonAdd(buttons, "Обновить");
    TgBotResponseSend(token, from, "<b>ВЫБОР ОБЪЕКТА</b>\n\nДобро пожаловать в Умный Дом", buttons);
}

void MainMenuProcess(const char *token, unsigned from, const char *message)
{
    char        text[EXT_STR_LEN];
    json_t      *buttons = json_array();
    StackUnit   *unit = TgMenuUnitGet(from);
    const char  *line0[] = {"Метео", "Охрана"};
    const char  *line1[] = {"Розетки", "Назад"};

    snprintf(text, EXT_STR_LEN, "<b>ГЛАВНОЕ МЕНЮ: %s</b>\n", unit->name);

    TgBotButtonsAdd(buttons, 2, line0);
    TgBotButtonsAdd(buttons, 2, line1);
    TgBotResponseSend(token, from, text, buttons);
}

void SocketMenuProcess(const char *token, unsigned from, const char *message)
{
    char        text[EXT_STR_LEN];
    char        sock_text[STR_LEN];
    char        status_text[STR_LEN];
    GList       *sockets = NULL;
    bool        ret = false;
    bool        status = false;
    json_t      *buttons = json_array();
    StackUnit   *unit = TgMenuUnitGet(from);
    const char  *line_last[] = {"Обновить", "Назад"};

    snprintf(text, EXT_STR_LEN, "<b>РОЗЕТКИ: %s</b>\n\n", unit->name);

    if (strcmp(message, "Розетки") && strcmp(message, "Назад") && strcmp(message, "Обновить")) {
        if (!RpcSocketStatusGet(unit->id, message, &status)) {
            strcat(text, "<b>Ошибка получения статуса розетки</b>\n\n");
        }

        if (!RpcSocketStatusSet(unit->id, message, !status)) {
            strcat(text, "<b>Ошибка переключения розетки</b>\n\n");
        }
    }

    ret = RpcSocketsGet(unit->id, &sockets);

    if (ret) {
        for (GList *s = sockets; s != NULL; s = s->next) {
            RpcSocket *socket = (RpcSocket *)s->data;

            TgBotButtonAdd(buttons, socket->name);

            if (socket->status) {
                strncpy(status_text, "Включен", STR_LEN);
            } else {
                strncpy(status_text, "Отключен", STR_LEN);
            }

            snprintf(sock_text, STR_LEN, "        %s: <b>%s</b>\n", socket->name, status_text);
            strcat(text, sock_text);
            free(socket);
        }
        g_list_free(sockets);
    } else {
        strcat(text, "<b>Ошибка</b>");
    }

    TgBotButtonsAdd(buttons, 2, line_last);
    TgBotResponseSend(token, from, text, buttons);
}

void MeteoMenuProcess(const char *token, unsigned from, const char *message)
{
    char        text[EXT_STR_LEN];
    char        sensor_text[STR_LEN];
    GList       *sensors = NULL;
    bool        ret = false;
    json_t      *buttons = json_array();
    StackUnit   *unit = TgMenuUnitGet(from);
    const char  *line_last[] = {"Обновить", "Назад"};

    snprintf(text, EXT_STR_LEN, "<b>МЕТЕО: %s</b>\n\n", unit->name);

    ret = RpcMeteoSensorsGet(unit->id, &sensors);

    if (ret) {
        for (GList *s = sensors; s != NULL; s = s->next) {
            RpcMeteoSensor *sensor = (RpcMeteoSensor *)s->data;

            switch (sensor->type) {
                case RPC_METEO_SENSOR_DS18B20:
                    snprintf(sensor_text, STR_LEN, "        %s: <b>%.1f°</b>\n", sensor->name, sensor->ds18b20.temp);
                    break;
            }

            strcat(text, sensor_text);
            free(sensor);
        }
        g_list_free(sensors);
    } else {
        strcat(text, "<b>Ошибка</b>");
    }

    TgBotButtonsAdd(buttons, 2, line_last);
    TgBotResponseSend(token, from, text, buttons);
}

void SecurityMenuProcess(const char *token, unsigned from, const char *message)
{
    char        text[EXT_STR_LEN];
    char        status_text[STR_LEN];
    char        name_text[STR_LEN];
    char        siren_text[STR_LEN];
    bool        ret = false;
    bool        status = false;
    bool        alarm = false;
    GList       *sensors = NULL;
    StackUnit   *unit = TgMenuUnitGet(from);
    json_t      *buttons = json_array();
    const char  *line_last[] = {"Обновить", "Назад"};

    if (!strcmp(message, "Включить")) {
        if (!RpcSecurityStatusSet(unit->id, true)) {
            LogF(LOG_TYPE_ERROR, "TGHANDLERS", "Failed to enable security unit \"%d\" status", unit);
        }
    } else if (!strcmp(message, "Отключить")) {
        if (!RpcSecurityStatusSet(unit->id, false)) {
            LogF(LOG_TYPE_ERROR, "TGHANDLERS", "Failed to disable security unit \"%d\" status", unit);
        }
    } else if (!strcmp(message, "Сирена")) {
        if (!RpcSecurityAlarmSet(unit->id, true)) {
            LogF(LOG_TYPE_ERROR, "TGHANDLERS", "Failed to disable alarm of security unit \"%d\" status", unit);
        }
    }

    ret = RpcSecurityStatusGet(unit->id, &status);

    if (ret) {
        if (status) {
            TgBotButtonAdd(buttons, "Отключить");
            strncpy(status_text, "Работает", STR_LEN);
        } else {
            TgBotButtonAdd(buttons, "Включить");
            strncpy(status_text, "Отключен", STR_LEN);
        }
    } else {
        strncpy(status_text, "Ошибка", STR_LEN);
    }

    ret = RpcSecurityAlarmGet(unit->id, &alarm);

    if (ret) {
        if (alarm) {
            strncpy(siren_text, "Работает", STR_LEN);
        } else {
            strncpy(siren_text, "Отключена", STR_LEN);
        }
    } else {
        strncpy(siren_text, "Ошибка", STR_LEN);
    }

    snprintf(text, STR_LEN, "<b>ОХРАНА: %s</b>\n\n        %s: <b>%s</b>\n        %s: <b>%s</b>\n\n<b>Датчики:</b>\n\n",
            unit->name, "Статус", status_text, "Сирена", siren_text);

    ret = RpcSecuritySensorsGet(unit->id, &sensors);

    if (ret) {
        for (GList *c = sensors; c != NULL; c = c->next) {
            RpcSecuritySensor *sensor = (RpcSecuritySensor *)c->data;

            snprintf(name_text, STR_LEN, "        %s : <b>", sensor->name);
            strcat(text, name_text);

            if (sensor->detected) {
                if (sensor->type == RPC_SECURITY_SENSOR_REED) {
                    strcat(text, "Открыт</b>\n");
                } else {
                    strcat(text, "Движение</b>\n");
                }
            } else {
                if (sensor->type == RPC_SECURITY_SENSOR_REED) {
                    strcat(text, "Закрыт</b>\n");
                } else {
                    strcat(text, "Никого</b>\n");
                }
            }

            free(sensor);
        }
        g_list_free(sensors);
    } else {
        strcat(text, "<b>Ошибка</b>\n");
    }

    TgBotButtonAdd(buttons, "Сирена");
    TgBotButtonsAdd(buttons, 2, line_last);
    TgBotResponseSend(token, from, text, buttons);
}
