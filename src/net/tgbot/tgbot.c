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

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static char     token[STR_LEN] = {0};
static GList    *users = NULL;
static unsigned message_id = 0;

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static void MessageProcess(unsigned from, const char *message)
{
    bool found = false;

    for (GList *u = users; u != NULL; u = u->next) {
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
        case TG_MENU_LVL_STACK_SELECT:
            if (strcmp(message, "Назад")) {
                if (StackUnitNameCheck(message)) {
                    TgMenuUnitSet(from, StackUnitIdGet(message));
                    TgMenuLevelSet(from, TG_MENU_LVL_MAIN);
                }
            }
            break;

        case TG_MENU_LVL_MAIN:
            if (!strcmp(message, "Охрана")) {
                TgMenuLevelSet(from, TG_MENU_LVL_SECURITY);
            } else if (!strcmp(message, "Метео")) {
                TgMenuLevelSet(from, TG_MENU_LVL_METEO);
            } else if (!strcmp(message, "Розетки")) {
                TgMenuLevelSet(from, TG_MENU_LVL_SOCKET);
            }
            break;
    }

    switch (TgMenuLevelGet(from)) {
        case TG_MENU_LVL_STACK_SELECT:
            StackSelectMenuProcess(token, from, message);
            break;

        case TG_MENU_LVL_MAIN:
            MainMenuProcess(token, from, message);
            break;

        case TG_MENU_LVL_SOCKET:
            SocketMenuProcess(token, from, message);
            break;

        case TG_MENU_LVL_SECURITY:
            SecurityMenuProcess(token, from, message);
            break;

        case TG_MENU_LVL_METEO:
            MeteoMenuProcess(token, from, message);
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
        snprintf(url, STR_LEN, "https://api.telegram.org/bot%s/getUpdates?offset=-1", token);

        if (WebClientRequest(WEB_REQ_GET, url, NULL, buf)) {
            json_t *root = json_loads(buf, 0, &error);
            if (root == NULL) {
                LogF(LOG_TYPE_ERROR, "TGBOT", "Failed to parse telegram request: %s", error.text);
                thrd_sleep(&(struct timespec){ .tv_sec = 1 }, NULL);
                continue;
            }

            json_array_foreach(json_object_get(root, "result"), index, value) {
                json_t *message = json_object_get(value, "message");
                json_t *from = json_object_get(message, "from");
                int id = json_integer_value(json_object_get(message, "message_id"));

                if (id != message_id) {
                    message_id = id;
                    MessageProcess(json_integer_value(json_object_get(from, "id")),
                                    json_string_value(json_object_get(message, "text")));
                }
            }

            json_decref(root);
        }

        thrd_sleep(&(struct timespec){ .tv_sec = 1 }, NULL);
    }

    return 0;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void TgBotUserAdd(TgBotUser *user)
{
    users = g_list_append(users, (void *)user);
}

void TgBotTokenSet(const char *bot_token)
{
    strncpy(token, bot_token, STR_LEN);
}

bool TgBotStart()
{
    thrd_t  th;

    thrd_create(&th, &TelegramThread, NULL);

    return true;
}
