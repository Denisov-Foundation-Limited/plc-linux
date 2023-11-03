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

static bool TgBotResponseSend(const char *token, unsigned id, const char *text, const char *markup)
{
    char    url[STR_LEN];
    char    post[EXT_STR_LEN];
    char    buf[BUFFER_LEN_MAX];

    snprintf(url, STR_LEN, "https://api.telegram.org/bot%s/sendMessage", token);
    snprintf(post, EXT_STR_LEN, "chat_id=%d&parse_mode=HTML&text=%s&reply_markup={\"keyboard\":%s}", id, text, markup);

    return WebClientRequest(WEB_REQ_POST, url, post, buf);
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void StackSelectMenuProcess(const char *token, unsigned from, const char *message)
{
    char    buttons[EXT_STR_LEN];
    char    button[STR_LEN];
    GList   *units = NULL;

    StackActiveUnitsGet(&units);

    strncpy(buttons, "[", EXT_STR_LEN);

    for (GList *u = units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;
        snprintf(button, STR_LEN, "[\"%s\"],", unit->name);
        strcat(buttons, button);
    }
    g_list_free(units);

    strcat(buttons, "[\"Обновить\"]]");
    TgBotResponseSend(token, from, "<b>ВЫБОР ОБЪЕКТА</b>\n\nДобро пожаловать в Умный Дом", buttons);
}

void MainMenuProcess(const char *token, unsigned from, const char *message)
{
    TgBotResponseSend(token, from, "<b>ГЛАВНОЕ МЕНЮ</b>\n\n",
                        "[[\"Метео\"],[\"Охрана\"],[\"Розетки\"],[\"Назад\"]]");
}

void SocketMenuProcess(const char *token, unsigned from, const char *message)
{
    char    text[EXT_STR_LEN];
    char    sock_text[STR_LEN];
    char    buttons[EXT_STR_LEN];
    char    button[STR_LEN];
    GList   *sockets = NULL;
    bool    ret = false;

    unsigned unit = TgMenuUnitGet(from);

    strncpy(text, "<b>РОЗЕТКИ</b>\n\n", EXT_STR_LEN);
    strncpy(buttons, "[", EXT_STR_LEN);

    if (strcmp(message, "Розетки") && strcmp(message, "Назад") && strcmp(message, "Обновить")) {
        if (!RpcSocketStatusSet(unit, message, !RpcSocketStatusGet(unit, message))) {
            strcat(text, "<b>Ошибка переключения розетки</b>\n\n");
        }
    }

    ret = RpcSocketsGet(unit, &sockets);

    if (ret) {
        for (GList *s = sockets; s != NULL; s = s->next) {
            RpcSocket *socket = (RpcSocket *)s->data;

            snprintf(button, STR_LEN, "[\"%s\"],", socket->name);
            strcat(buttons, button);

            if (RpcSocketStatusGet(unit, socket->name)) {
                snprintf(sock_text, STR_LEN, "        %s: <b>Включен</b>\n", socket->name);
            } else {
                snprintf(sock_text, STR_LEN, "        %s: <b>Отключен</b>\n", socket->name);
            }

            strcat(text, sock_text);
            free(socket);
        }
        g_list_free(sockets);
    } else {
        strcat(text, "<b>Ошибка</b>");
    }
    strcat(buttons, "[\"Обновить\",\"Назад\"]]");

    TgBotResponseSend(token, from, text, buttons);
}

void MeteoMenuProcess(const char *token, unsigned from, const char *message)
{
    char    text[EXT_STR_LEN];
    char    sensor_text[STR_LEN];
    char    buttons[EXT_STR_LEN];
    GList   *sensors = NULL;
    bool    ret = false;

    unsigned unit = TgMenuUnitGet(from);

    strncpy(text, "<b>МЕТЕО</b>\n\n", EXT_STR_LEN);

    ret = RpcMeteoSensorsGet(unit, &sensors);

    if (ret) {
        for (GList *s = sensors; s != NULL; s = s->next) {
            RpcMeteoSensor *sensor = (RpcMeteoSensor *)s->data;
            snprintf(sensor_text, STR_LEN, "        %s: <b>%.1f°</b>\n", sensor->name, sensor->temp);
            strcat(text, sensor_text);
            free(sensor);
        }
        g_list_free(sensors);
    } else {
        strcat(text, "<b>Ошибка</b>");
    }

    TgBotResponseSend(token, from, text, "[[\"Обновить\"],[\"Назад\"]]");
}

void SecurityMenuProcess(const char *token, unsigned from, const char *message)
{
    char    text[EXT_STR_LEN];
    char    status_text[STR_LEN];
    char    buttons[STR_LEN];
    char    siren_text[STR_LEN];
    bool    ret = false;
    bool    status = false;
    bool    alarm = false;
    GList   *sensors = NULL;

    unsigned unit = TgMenuUnitGet(from);

    if (!strcmp(message, "Включить")) {
        if (!RpcSecurityStatusSet(unit, true)) {
            LogF(LOG_TYPE_ERROR, "TGHANDLERS", "Failed to enable security unit \"%d\" status", unit);
        }
    } else if (!strcmp(message, "Отключить")) {
        if (!RpcSecurityStatusSet(unit, false)) {
            LogF(LOG_TYPE_ERROR, "TGHANDLERS", "Failed to disable security unit \"%d\" status", unit);
        }
    } else if (!strcmp(message, "Сирена")) {
        if (!RpcSecurityAlarmSet(unit, true)) {
            LogF(LOG_TYPE_ERROR, "TGHANDLERS", "Failed to disable alarm of security unit \"%d\" status", unit);
        }
    }

    ret = RpcSecurityStatusGet(unit, &status);

    if (ret) {
        if (status) {
            strncpy(status_text, "Работает", STR_LEN);
            strncpy(buttons, "[[\"Отключить\"], [\"Сирена\"], [\"Обновить\", \"Назад\"]]", STR_LEN);
        } else {
            strncpy(status_text, "Отключен", STR_LEN);
            strncpy(buttons, "[[\"Включить\"], [\"Сирена\"], [\"Обновить\", \"Назад\"]]", STR_LEN);
        }
    } else {
        strncpy(status_text, "Ошибка", STR_LEN);
        strncpy(buttons, "[[\"Сирена\"], [\"Обновить\", \"Назад\"]]", STR_LEN);
    }

    ret = RpcSecurityAlarmGet(unit, &alarm);

    if (ret) {
        if (alarm) {
            strncpy(siren_text, "Работает", STR_LEN);
        } else {
            strncpy(siren_text, "Отключена", STR_LEN);
        }
    } else {
        strncpy(siren_text, "Ошибка", STR_LEN);
    }

    snprintf(text, STR_LEN, "<b>ОХРАНА</b>\n\n        Статус: <b>%s</b>\n        Сирена: <b>%s</b>\n\n<b>Датчики:</b>\n\n",
            status_text, siren_text);

    ret = RpcSecuritySensorsGet(unit, &sensors);

    if (ret) {
        for (GList *c = sensors; c != NULL; c = c->next) {
            RpcSecuritySensor *sensor = (RpcSecuritySensor *)c->data;

            strcat(text, "        ");
            strcat(text, sensor->name);
            strcat(text, ": <b>");

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
                    strcat(text, "Нет движения</b>\n");
                }
            }

            free(sensor);
        }
        g_list_free(sensors);
    } else {
        strcat(text, "<b>Ошибка</b>\n");
    }

    TgBotResponseSend(token, from, text, buttons);
}
