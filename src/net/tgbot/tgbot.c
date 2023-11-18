/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <threads.h>

#include <jansson.h>
#include <glib-2.0/glib.h>

#include <net/tgbot/tgbot.h>
#include <net/tgbot/tgmenu.h>
#include <net/tgbot/tghandlers.h>
#include <net/web/webclient.h>
#include <utils/log.h>
#include <stack/stack.h>

#include <net/tgbot/handlers/tgsocket.h>
#include <net/tgbot/handlers/tgsecurity.h>
#include <net/tgbot/handlers/tgmeteo.h>
#include <net/tgbot/handlers/tgmain.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct {
    char        token[STR_LEN];
    GList       *users;
    unsigned    message_id;
    bool        enabled;
} TgBot = {
    .users = NULL,
    .message_id = 0,
    .enabled = true
};

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static void MessageProcess(unsigned from, const char *message)
{
    bool found = false;

    for (GList *u = TgBot.users; u != NULL; u = u->next) {
        TgBotUser *user = (TgBotUser *)u->data;
        if (user->chat_id == from) {
            LogF(LOG_TYPE_INFO, "TGBOT", "User \"%s\" authorized with msg \"%s\"", user->name, message);
            found = true;
            break;
        }
    }

    if (!found) {
        LogF(LOG_TYPE_INFO, "TGBOT", "Invalid user \"%d\" not authorized", from);
        return;
    }

    if (!strcmp(message, "Назад")) {
        TgMenuBack(from);
    }

    switch (TgMenuLevelGet(from)) {
        case TG_MENU_LVL_MAIN:
            if (!strcmp(message, "Охрана")) {
                TgMenuLevelSet(from, TG_MENU_LVL_SECURITY);
            } else if (!strcmp(message, "Метео")) {
                TgMenuLevelSet(from, TG_MENU_LVL_METEO);
            } else if (!strcmp(message, "Розетки")) {
                TgMenuLevelSet(from, TG_MENU_LVL_SOCKET_SELECT);
            }
            break;

        case TG_MENU_LVL_SOCKET_SELECT:
            if (strcmp(message, "Назад")) {
                if (StackUnitNameCheck(message)) {
                    TgMenuUnitSet(from, StackUnitNameGet(message));
                    TgMenuLevelSet(from, TG_MENU_LVL_SOCKET);
                }
            }
            break;
    }

    switch (TgMenuLevelGet(from)) {
        case TG_MENU_LVL_MAIN:
            TgMainProcess(TgBot.token, from, message);
            break;

        case TG_MENU_LVL_SOCKET_SELECT:
            TgSocketSelectProcess(TgBot.token, from, message);
            break;

        case TG_MENU_LVL_SOCKET:
            TgSocketProcess(TgBot.token, from, message);
            break;

        case TG_MENU_LVL_SECURITY:
            TgSecurityProcess(TgBot.token, from, message);
            break;

        case TG_MENU_LVL_METEO:
            TgMeteoProcess(TgBot.token, from, message);
            break;
    }
}

static int TelegramThread(void *data)
{
    char            buf[BUFFER_LEN_MAX];
    char            url[STR_LEN];
    json_error_t    error;
    size_t          index;
    json_t          *value;

    for (;;) {
        memset(buf, 0x0, BUFFER_LEN_MAX);
        snprintf(url, STR_LEN, "https://api.telegram.org/bot%s/getUpdates?offset=-1", TgBot.token);

        if (WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
            json_t *root = json_loads(buf, 0, &error);

            if (root == NULL) {
                LogF(LOG_TYPE_ERROR, "TGBOT", "Failed to parse telegram request: %s", error.text);
                UtilsSecSleep(1);
                continue;
            }

            json_array_foreach(json_object_get(root, "result"), index, value) {
                json_t *message = json_object_get(value, "message");
                json_t *from = json_object_get(message, "from");
                json_t *msg_id = json_object_get(message, "message_id");

                if (message == NULL || from == NULL || msg_id == NULL) {
                    Log(LOG_TYPE_ERROR, "TGBOT", "Failed to parse message & from & msg_id");
                    UtilsSecSleep(1);
                    continue;
                }

                json_t *from_id = json_object_get(from, "id");
                json_t *text = json_object_get(message, "text");

                if (text == NULL || from_id == NULL) {
                    Log(LOG_TYPE_ERROR, "TGBOT", "Failed to parse text & from_id");
                    UtilsSecSleep(1);
                    continue;
                }

                int id = json_integer_value(msg_id);

                if (id != TgBot.message_id) {
                    TgBot.message_id = id;
                    MessageProcess(json_integer_value(from_id), json_string_value(text));
                }
            }

            json_decref(root);
        }

        UtilsSecSleep(1);
    }

    return 0;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void TgBotDisable()
{
    TgBot.enabled = false;
}

void TgBotUserAdd(TgBotUser *user)
{
    TgBot.users = g_list_append(TgBot.users, (void *)user);
}

void TgBotTokenSet(const char *bot_token)
{
    strncpy(TgBot.token, bot_token, STR_LEN);
}

bool TgBotStart()
{
    thrd_t  th;

    if (!TgBot.enabled) {
        return true;
    }

    thrd_create(&th, &TelegramThread, NULL);

    return true;
}
