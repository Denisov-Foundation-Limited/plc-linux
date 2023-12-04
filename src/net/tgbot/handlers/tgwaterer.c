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
#include <net/tgbot/handlers/tgwaterer.h>
#include <net/tgbot/tgresp.h>
#include <net/tgbot/tgmenu.h>
#include <utils/log.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void TgWatererStackSelectProcess(const char *token, unsigned from, const char *message)
{
    json_t      *buttons = json_array();
    GList       *units = NULL;
    GList       *wtrs = NULL;
    const char  *line_last[] = {"Обновить", "Назад"};
    GString     *text = g_string_new("<b>ПОЛИВ: ВЫБОР ОБЪЕКТА</b>\n\n");

    StackActiveUnitsGet(&units);

    for (GList *u = units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;

        if (RpcWaterersGet(unit->id, &wtrs)) {
            if (g_list_length(wtrs) > 0) {
                g_string_append_printf(text, "<b>%s:</b>\n", unit->name);
            }

            for (GList *s = wtrs; s != NULL; s = s->next) {
                RpcWaterer *wtr = (RpcWaterer *)s->data;

                g_string_append_printf(text, "        <b>%s:</b>\n", wtr->name);

                if (wtr->status) {
                    g_string_append_printf(text, "                Статус: <b>Включен</b>\n");
                } else {
                    g_string_append_printf(text, "                Статус: <b>Отключен</b>\n");
                }

                if (wtr->valve) {
                    g_string_append_printf(text, "                Кран: <b>Открыт</b>\n");
                } else {
                    g_string_append_printf(text, "                Кран: <b>Закрыт</b>\n");
                }

                for (GList *t = wtr->times; t != NULL; t = t->next) {
                    RpcWatererTime *tm = (RpcWatererTime *)t->data;
                    free(tm);
                }

                g_list_free(wtr->times);
                free(wtr);
            }
            g_list_free(wtrs);
            wtrs = NULL;
        } else {
            text = g_string_append(text, "Ошибка получения списка поливающих устройств");
            LogF(LOG_TYPE_ERROR, "TGWATERER", "Failed to get wtrs list for user %d", from);
        }

        TgRespButtonAdd(buttons, unit->name);
    }
    g_list_free(units);

    TgRespButtonsAdd(buttons, 2, line_last);
    TgRespSend(token, from, text->str, buttons);
    g_string_free(text, true);
}

void TgWatererSelectProcess(const char *token, unsigned from, const char *message)
{
    json_t      *buttons = json_array();
    GList       *wtrs = NULL;
    StackUnit   *unit = TgMenuUnitGet(from);
    GString     *text = g_string_new("");
    char        day[SHORT_STR_LEN];
    char        state[SHORT_STR_LEN];

    g_string_append_printf(text, "<b>ПОЛИВ: %s</b>\n\n", unit->name);

    if (RpcWaterersGet(unit->id, &wtrs)) {
        for (GList *s = wtrs; s != NULL; s = s->next) {
            RpcWaterer *wtr = (RpcWaterer *)s->data;

            g_string_append_printf(text, "<b>%s:</b>\n", wtr->name);

            if (wtr->status) {
                g_string_append_printf(text, "        Статус: <b>Включен</b>\n");
            } else {
                g_string_append_printf(text, "        Статус: <b>Отключен</b>\n");
            }

            if (wtr->valve) {
                g_string_append_printf(text, "        Кран: <b>Открыт</b>\n");
            } else {
                g_string_append_printf(text, "        Кран: <b>Закрыт</b>\n");
            }

            g_string_append_printf(text, "        \n<b>Расписание:</b>\n");
            for (GList *t = wtr->times; t != NULL; t = t->next) {
                RpcWatererTime *tm = (RpcWatererTime *)t->data;

                switch (tm->day) {
                    case 0:
                        strncpy(day, "ПН", SHORT_STR_LEN);
                        break;

                    case 1:
                        strncpy(day, "ВТ", SHORT_STR_LEN);
                        break;

                    case 2:
                        strncpy(day, "СР", SHORT_STR_LEN);
                        break;

                    case 3:
                        strncpy(day, "ЧТ", SHORT_STR_LEN);
                        break;

                    case 4:
                        strncpy(day, "ПТ", SHORT_STR_LEN);
                        break;

                    case 5:
                        strncpy(day, "СБ", SHORT_STR_LEN);
                        break;

                    case 6:
                        strncpy(day, "ВС", SHORT_STR_LEN);
                        break;
                }

                if (tm->state) {
                    strncpy(state, "Открыть", SHORT_STR_LEN);
                } else {
                    strncpy(state, "Закрыть", SHORT_STR_LEN);
                }

                g_string_append_printf(text, "                День:<b>%s</b> Час: <b>%d</b> Мин: <b>%d</b> Кран: <b>%s</b>\n",
                    day, tm->hour, tm->min, state);

                free(tm);
            }

            TgRespButtonAdd(buttons, wtr->name);

            free(wtr);
        }
        g_list_free(wtrs);
        wtrs = NULL;
    } else {
        text = g_string_append(text, "Ошибка получения списка поливающих устройств");
        LogF(LOG_TYPE_ERROR, "TGWATERER", "Failed to get wtrs list for user %d", from);
    }

    TgRespButtonAdd(buttons, "Назад");
    TgRespSend(token, from, text->str, buttons);
    g_string_free(text, true);
}

void TgWatererProcess(const char *token, unsigned from, const char *message)
{
    json_t      *buttons = json_array();
    StackUnit   *unit = TgMenuUnitGet(from);
    const char  *wtr_name = TgMenuDataGet(from);
    const char  *line_last[] = {"Обновить", "Назад"};
    GString     *text = g_string_new("");
    GList       *wtrs = NULL;
    char        day[SHORT_STR_LEN];
    char        state[SHORT_STR_LEN];

    if (!strcmp(message, "Включить")) {
        if (!RpcWatererStatusSet(unit->id, wtr_name, true)) {
            g_string_append_printf(text, "        <b>Ошибка включения Полива</b>\n");
            LogF(LOG_TYPE_ERROR, "TGWATERER", "Failed to enable wtr water control for user %d", from);
        }
    } else if (!strcmp(message, "Отключить")) {
        if (!RpcWatererStatusSet(unit->id, wtr_name, false)) {
            g_string_append_printf(text, "        <b>Ошибка отключения Полива</b>\n");
            LogF(LOG_TYPE_ERROR, "TGWATERER", "Failed to disable wtr water control for user %d", from);
        }
    } else if (!strcmp(message, "Кран Закрыть")) {
        if (!RpcWatererValveSet(unit->id, wtr_name, false)) {
            g_string_append_printf(text, "        <b>Ошибка закрытия крана</b>\n");
            LogF(LOG_TYPE_ERROR, "TGWATERER", "Failed to disable pump of wtr water control for user %d", from);
        }
    } else if (!strcmp(message, "Кран Открыть")) {
        if (!RpcWatererValveSet(unit->id, wtr_name, true)) {
            g_string_append_printf(text, "        <b>Ошибка открытия крана</b>\n");
            LogF(LOG_TYPE_ERROR, "TGWATERER", "Failed to enable pump of wtr water control for user %d", from);
        }
    }

    if (RpcWaterersGet(unit->id, &wtrs)) {
        for (GList *s = wtrs; s != NULL; s = s->next) {
            RpcWaterer *wtr = (RpcWaterer *)s->data;

            if (!strcmp(wtr->name, wtr_name)) {
                g_string_append_printf(text, "<b>БАК: %s: %s</b>\n", unit->name, wtr->name);

                if (wtr->status) {
                    TgRespButtonAdd(buttons, "Отключить");
                    g_string_append_printf(text, "        Статус: <b>Включен</b>\n");
                } else {
                    TgRespButtonAdd(buttons, "Включить");
                    g_string_append_printf(text, "        Статус: <b>Отключен</b>\n");
                }

                if (wtr->valve) {
                    g_string_append_printf(text, "        Кран: <b>Открыт</b>\n");
                    TgRespButtonAdd(buttons, "Кран Закрыть");
                } else {
                    g_string_append_printf(text, "        Кран: <b>Закрыт</b>\n");
                    TgRespButtonAdd(buttons, "Кран Открыть");
                }
            }

            g_string_append_printf(text, "        \n<b>Расписание:</b>\n");
            for (GList *t = wtr->times; t != NULL; t = t->next) {
                RpcWatererTime *tm = (RpcWatererTime *)t->data;

                switch (tm->day) {
                    case 0:
                        strncpy(day, "ПН", SHORT_STR_LEN);
                        break;

                    case 1:
                        strncpy(day, "ВТ", SHORT_STR_LEN);
                        break;

                    case 2:
                        strncpy(day, "СР", SHORT_STR_LEN);
                        break;

                    case 3:
                        strncpy(day, "ЧТ", SHORT_STR_LEN);
                        break;

                    case 4:
                        strncpy(day, "ПТ", SHORT_STR_LEN);
                        break;

                    case 5:
                        strncpy(day, "СБ", SHORT_STR_LEN);
                        break;

                    case 6:
                        strncpy(day, "ВС", SHORT_STR_LEN);
                        break;
                }

                if (tm->state) {
                    strncpy(state, "Открыть", SHORT_STR_LEN);
                } else {
                    strncpy(state, "Закрыть", SHORT_STR_LEN);
                }

                g_string_append_printf(text, "                День:<b>%s</b> Час: <b>%d</b> Мин: <b>%d</b> Кран: <b>%s</b>\n",
                    day, tm->hour, tm->min, state);

                free(tm);
            }

            g_list_free(wtr->times);
            free(wtr);
        }
        g_list_free(wtrs);
        wtrs = NULL;
    } else {
        text = g_string_append(text, "Ошибка получения списка поливающих устройств");
        LogF(LOG_TYPE_ERROR, "TGWATERER", "Failed to get wtrs list for user %d", from);
    }

    TgRespButtonsAdd(buttons, 2, line_last);
    TgRespSend(token, from, text->str, buttons);
    g_string_free(text, true);
}
