/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <jansson.h>

#include <utils/utils.h>
#include <stack/stack.h>
#include <stack/rpc.h>
#include <net/tgbot/handlers/tgsecurity.h>
#include <net/tgbot/tgresp.h>
#include <net/tgbot/tgmenu.h>
#include <utils/log.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void TgSecurityProcess(const char *token, unsigned from, const char *message)
{
    bool        status = false;
    bool        alarm = false;
    GList       *sensors = NULL;
    GList       *units = NULL;
    json_t      *buttons = json_array();
    const char  *line_last[] = {"Обновить", "Назад"};
    GString     *text = g_string_new("");

    if (!strcmp(message, "Включить")) {
        if (!RpcSecurityStatusSet(RPC_DEFAULT_UNIT, true)) {
            LogF(LOG_TYPE_ERROR, "TGSECURITY", "Failed to enable security status for user \"%d\"", from);
        }
    } else if (!strcmp(message, "Отключить")) {
        if (!RpcSecurityStatusSet(RPC_DEFAULT_UNIT, false)) {
            LogF(LOG_TYPE_ERROR, "TGSECURITY", "Failed to disable security status for user \"%d\"", from);
        }
    } else if (!strcmp(message, "Сирена")) {
        if (!RpcSecurityAlarmSet(RPC_DEFAULT_UNIT, true)) {
           LogF(LOG_TYPE_ERROR, "TGSECURITY", "Failed to enable security siren for user \"%d\"", from);
        }
    }

    text = g_string_append(text, "<b>ОХРАНА</b>\n        Статус: <b>");

    if (RpcSecurityStatusGet(RPC_DEFAULT_UNIT, &status)) {
        if (status) {
            TgRespButtonAdd(buttons, "Отключить");
            text = g_string_append(text, "Работает</b>\n");
        } else {
            TgRespButtonAdd(buttons, "Включить");
            text = g_string_append(text, "Отключен</b>\n");
        }
    } else {
        text = g_string_append(text, "Ошибка</b>\n");
        LogF(LOG_TYPE_ERROR, "TGSECURITY", "Failed to get security status for user \"%d\"", from);
    }

    text = g_string_append(text, "        Сирена: <b>");

    if (RpcSecurityAlarmGet(RPC_DEFAULT_UNIT, &alarm)) {
        if (alarm) {
            text = g_string_append(text, "Работает</b>\n");
        } else {
            text = g_string_append(text, "Отключена</b>\n");
        }
    } else {
        text = g_string_append(text, "Ошибка</b>\n\n");
        LogF(LOG_TYPE_ERROR, "TGSECURITY", "Failed to get security siren for user \"%d\"", from);
    }

    StackActiveUnitsGet(&units);

    for (GList *u = units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;

        g_string_append_printf(text, "<b>%s</b>\n", unit->name);

        if (RpcSecuritySensorsGet(unit->id, &sensors)) {
            for (GList *c = sensors; c != NULL; c = c->next) {
                RpcSecuritySensor *sensor = (RpcSecuritySensor *)c->data;

                if (sensor->detected) {
                    if (sensor->type == RPC_SECURITY_SENSOR_REED) {
                        g_string_append_printf(text, "        %s: <b>Открыт</b>\n", sensor->name);
                    } else {
                        g_string_append_printf(text, "        %s: <b>Движение</b>\n", sensor->name);
                    }
                } else {
                    if (sensor->type == RPC_SECURITY_SENSOR_REED) {
                        g_string_append_printf(text, "        %s: <b>Закрыт</b>\n", sensor->name);
                    } else {
                        g_string_append_printf(text, "        %s: <b>Никого</b>\n", sensor->name);
                    }
                }

                free(sensor);
            }
            g_list_free(sensors);
            sensors = NULL;
        } else {
            g_string_append_printf(text, "        <b>Ошибка</b>\n");
            LogF(LOG_TYPE_ERROR, "TGSECURITY", "Failed to get security sensors for user \"%d\" and unit \"%s\"", from, unit->name);
        }
    }
    g_list_free(units);

    TgRespButtonAdd(buttons, "Сирена");
    TgRespButtonsAdd(buttons, 2, line_last);
    TgRespSend(token, from, text->str, buttons);
    g_string_free(text, true);
}
