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

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static void SocketSelectProcess(RpcSocketGroup group, const char *token, unsigned from, const char *message)
{
    json_t      *buttons = json_array();
    GList       *units = NULL;
    GList       *sockets = NULL;
    const char  *line_last[] = {"Обновить", "Назад"};
    GString     *text = g_string_new("");
    
    switch (group) {
        case RPC_SOCKET_GROUP_LIGHT:
            text = g_string_append(text, "<b>СВЕТ: ВЫБОР ОБЪЕКТА</b>\n\n");
            break;

        case RPC_SOCKET_GROUP_SOCKET:
            text = g_string_append(text, "<b>РОЗЕТКИ: ВЫБОР ОБЪЕКТА</b>\n\n");
            break;
    }

    StackActiveUnitsGet(&units);

    for (GList *u = units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;

        g_string_append_printf(text, "<b>%s:</b>\n", unit->name);

        if (RpcSocketsGet(unit->id, &sockets)) {
            for (GList *s = sockets; s != NULL; s = s->next) {
                RpcSocket *socket = (RpcSocket *)s->data;

                if (socket->group == group) {
                    if (socket->status) {
                        g_string_append_printf(text, "        %s: <b>Включен</b>\n", socket->name);
                    } else {
                        g_string_append_printf(text, "        %s: <b>Отключен</b>\n", socket->name);
                    }
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

void SocketProcess(RpcSocketGroup group, const char *token, unsigned from, const char *message)
{
    GList       *sockets = NULL;
    bool        status = false;
    json_t      *buttons = json_array();
    StackUnit   *unit = TgMenuUnitGet(from);
    const char  *line_last[] = {"Обновить", "Назад"};
    GString     *text = g_string_new("");

    switch (group) {
        case RPC_SOCKET_GROUP_LIGHT:
            g_string_append_printf(text, "<b>СВЕТ: %s</b>\n", unit->name);
            break;

        case RPC_SOCKET_GROUP_SOCKET:
            g_string_append_printf(text, "<b>РОЗЕТКИ: %s</b>\n", unit->name);
            break;
    }

    if (RpcSocketsGet(unit->id, &sockets)) {
        for (GList *s = sockets; s != NULL; s = s->next) {
            RpcSocket *socket = (RpcSocket *)s->data;

            if (!strcmp(socket->name, message)) {
                if (!RpcSocketStatusSet(unit->id, message, !socket->status)) {
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

            if (socket->group == group) {
                TgRespButtonAdd(buttons, socket->name);

                if (socket->status) {
                    g_string_append_printf(text, "        %s: <b>Включен</b>\n", socket->name);
                } else {
                    g_string_append_printf(text, "        %s: <b>Отключен</b>\n", socket->name);
                }
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

void TgSocketSelectProcess(const char *token, unsigned from, const char *message)
{
    SocketSelectProcess(RPC_SOCKET_GROUP_SOCKET, token, from, message);
}

void TgSocketProcess(const char *token, unsigned from, const char *message)
{
    SocketProcess(RPC_SOCKET_GROUP_SOCKET, token, from, message);
}

void TgLightSelectProcess(const char *token, unsigned from, const char *message)
{
    SocketSelectProcess(RPC_SOCKET_GROUP_LIGHT, token, from, message);
}

void TgLightProcess(const char *token, unsigned from, const char *message)
{
    SocketProcess(RPC_SOCKET_GROUP_LIGHT, token, from, message);
}