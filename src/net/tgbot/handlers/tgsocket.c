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
#include <net/tgbot/handlers/tgsocket.h>
#include <net/tgbot/tgresp.h>
#include <net/tgbot/tgmenu.h>
#include <utils/log.h>

void TgSocketSelectProcess(const char *token, unsigned from, const char *message)
{
    json_t      *buttons = json_array();
    GList       *units = NULL;
    GList       *sockets = NULL;
    const char  *line_last[] = {"Обновить", "Назад"};
    GString     *text = g_string_new("<b>РОЗЕТКИ: ВЫБОР ОБЪЕКТА</b>\n\n");

    StackActiveUnitsGet(&units);

    for (GList *u = units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;

        g_string_append_printf(text, "<b>%s</b>\n", unit->name);
        if (RpcSocketsGet(unit->id, &sockets)) {
            for (GList *s = sockets; s != NULL; s = s->next) {
                RpcSocket *socket = (RpcSocket *)s->data;

                if (socket->status) {
                    g_string_append_printf(text, "        %s: <b>Включен</b>\n", socket->name);
                } else {
                    g_string_append_printf(text, "        %s: <b>Отключен</b>\n", socket->name);
                }
                free(socket);
            }
            g_list_free(sockets);
            sockets = NULL;
        } else {
            text = g_string_append(text, "Ошибка получения списка розеток");
            LogF(LOG_TYPE_ERROR, "TGSOCKET", "Failed to get socket list for user %d", from);
        }

        TgRespButtonAdd(buttons, unit->name);
    }
    g_list_free(units);

    TgRespButtonsAdd(buttons, 2, line_last);
    TgRespSend(token, from, text->str, buttons);
    g_string_free(text, true);
}

void TgSocketProcess(const char *token, unsigned from, const char *message)
{
    GList       *sockets = NULL;
    bool        status = false;
    json_t      *buttons = json_array();
    StackUnit   *unit = TgMenuUnitGet(from);
    const char  *line_last[] = {"Обновить", "Назад"};
    GString     *text = g_string_new("");

    g_string_append_printf(text, "<b>РОЗЕТКИ: %s</b>\n", unit->name);

    if (RpcSocketsGet(unit->id, &sockets)) {
        for (GList *s = sockets; s != NULL; s = s->next) {
            RpcSocket *socket = (RpcSocket *)s->data;

            if (!strcmp(socket->name, message)) {
                if (!RpcSocketStatusGet(unit->id, message, &status)) {
                    text = g_string_append(text, "<b>Ошибка получения статуса розетки</b>\n\n");
                    LogF(LOG_TYPE_ERROR, "TGSOCKET", "Failed to get socket status for user %d", from);
                }
                if (!RpcSocketStatusSet(unit->id, message, !status)) {
                    text = g_string_append(text, "<b>Ошибка переключения розетки</b>\n\n");
                    LogF(LOG_TYPE_ERROR, "TGSOCKET", "Failed to set socket status for user %d", from);
                }
            }

            free(socket);
        }
        g_list_free(sockets);
        sockets = NULL;
    } else {
        text = g_string_append(text, "Ошибка переключения розетки");
        LogF(LOG_TYPE_ERROR, "TGSOCKET", "Failed to switch socket status for user %d", from);
    }

    if (RpcSocketsGet(unit->id, &sockets)) {
        for (GList *s = sockets; s != NULL; s = s->next) {
            RpcSocket *socket = (RpcSocket *)s->data;

            TgRespButtonAdd(buttons, socket->name);

            if (socket->status) {
                g_string_append_printf(text, "        %s: <b>Включен</b>\n", socket->name);
            } else {
                g_string_append_printf(text, "        %s: <b>Отключен</b>\n", socket->name);
            }

            free(socket);
        }
        g_list_free(sockets);
    } else {
        text = g_string_append(text, "Ошибка получения списка розеток");
        LogF(LOG_TYPE_ERROR, "TGSOCKET", "Failed to get socket list for user %d", from);
    }

    TgRespButtonsAdd(buttons, 2, line_last);
    TgRespSend(token, from, text->str, buttons);
    g_string_free(text, true);
}
