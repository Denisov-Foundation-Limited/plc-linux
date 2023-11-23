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
#include <net/tgbot/handlers/tgtank.h>
#include <net/tgbot/tgresp.h>
#include <net/tgbot/tgmenu.h>
#include <utils/log.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void TgTankStackSelectProcess(const char *token, unsigned from, const char *message)
{
    json_t      *buttons = json_array();
    GList       *units = NULL;
    GList       *tanks = NULL;
    const char  *line_last[] = {"Обновить", "Назад"};
    GString     *text = g_string_new("<b>БАК: ВЫБОР ОБЪЕКТА</b>\n\n");

    StackActiveUnitsGet(&units);

    for (GList *u = units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;

        g_string_append_printf(text, "<b>%s:</b>\n", unit->name);

        if (RpcTanksGet(unit->id, &tanks)) {
            for (GList *s = tanks; s != NULL; s = s->next) {
                RpcTank *tank = (RpcTank *)s->data;

                g_string_append_printf(text, "        <b>%s:</b>\n", tank->name);

                if (tank->status) {
                    g_string_append_printf(text, "                Статус: <b>Включен</b>\n");
                } else {
                    g_string_append_printf(text, "                Статус: <b>Отключен</b>\n");
                }

                g_string_append_printf(text, "                Уровень: <b>%u%%</b>\n", tank->level);

                if (tank->pump) {
                    g_string_append_printf(text, "                Насос: <b>Работает</b>\n");
                } else {
                    g_string_append_printf(text, "                Насос: <b>Отключен</b>\n");
                }

                if (tank->valve) {
                    g_string_append_printf(text, "                Набор воды: <b>Работает</b>\n");
                } else {
                    g_string_append_printf(text, "                Набор воды: <b>Отключен</b>\n");
                }

                free(tank);
            }
            g_list_free(tanks);
            tanks = NULL;
        } else {
            text = g_string_append(text, "Ошибка получения списка баков");
            LogF(LOG_TYPE_ERROR, "TGTANK", "Failed to get tanks list for user %d", from);
        }

        TgRespButtonAdd(buttons, unit->name);
    }
    g_list_free(units);

    TgRespButtonsAdd(buttons, 2, line_last);
    TgRespSend(token, from, text->str, buttons);
    g_string_free(text, true);
}

void TgTankSelectProcess(const char *token, unsigned from, const char *message)
{
    json_t      *buttons = json_array();
    GList       *tanks = NULL;
    StackUnit   *unit = TgMenuUnitGet(from);
    GString     *text = g_string_new("");

    g_string_append_printf(text, "<b>БАК: %s</b>\n\n", unit->name);

    if (RpcTanksGet(unit->id, &tanks)) {
        for (GList *s = tanks; s != NULL; s = s->next) {
            RpcTank *tank = (RpcTank *)s->data;

            g_string_append_printf(text, "<b>%s:</b>\n", tank->name);

            if (tank->status) {
                g_string_append_printf(text, "        Статус: <b>Включен</b>\n");
            } else {
                g_string_append_printf(text, "        Статус: <b>Отключен</b>\n");
            }

            g_string_append_printf(text, "        Уровень: <b>%u%%</b>\n", tank->level);

            if (tank->pump) {
                g_string_append_printf(text, "        Насос: <b>Работает</b>\n");
            } else {
                g_string_append_printf(text, "        Насос: <b>Отключен</b>\n");
            }

            if (tank->valve) {
                g_string_append_printf(text, "        Набор воды: <b>Работает</b>\n");
            } else {
                g_string_append_printf(text, "        Набор воды: <b>Отключен</b>\n");
            }

            TgRespButtonAdd(buttons, tank->name);

            free(tank);
        }
        g_list_free(tanks);
        tanks = NULL;
    } else {
        text = g_string_append(text, "Ошибка получения списка баков");
        LogF(LOG_TYPE_ERROR, "TGTANK", "Failed to get tanks list for user %d", from);
    }

    TgRespButtonAdd(buttons, "Назад");
    TgRespSend(token, from, text->str, buttons);
    g_string_free(text, true);
}

void TgTankProcess(const char *token, unsigned from, const char *message)
{
    json_t      *buttons = json_array();
    StackUnit   *unit = TgMenuUnitGet(from);
    const char  *tank_name = TgMenuDataGet(from);
    const char  *line_last[] = {"Обновить", "Назад"};
    GString     *text = g_string_new("");
    GList       *tanks = NULL;

    if (!strcmp(message, "Включить")) {
        if (!RpcTankStatusSet(unit->id, tank_name, true)) {
            g_string_append_printf(text, "        <b>Ошибка включения Бака</b>\n");
            LogF(LOG_TYPE_ERROR, "TGTANK", "Failed to enable tank water control for user %d", from);
        }
    } else if (!strcmp(message, "Отключить")) {
        if (!RpcTankStatusSet(unit->id, tank_name, false)) {
            g_string_append_printf(text, "        <b>Ошибка отключения Бака</b>\n");
            LogF(LOG_TYPE_ERROR, "TGTANK", "Failed to disable tank water control for user %d", from);
        }
    } else if (!strcmp(message, "Насос Отключить")) {
        if (!RpcTankPumpSet(unit->id, tank_name, false)) {
            g_string_append_printf(text, "        <b>Ошибка отключения Насоса Бака</b>\n");
            LogF(LOG_TYPE_ERROR, "TGTANK", "Failed to disable pump of tank water control for user %d", from);
        }
    } else if (!strcmp(message, "Насос Включить")) {
        if (!RpcTankPumpSet(unit->id, tank_name, true)) {
            g_string_append_printf(text, "        <b>Ошибка включения Насоса Бака</b>\n");
            LogF(LOG_TYPE_ERROR, "TGTANK", "Failed to enable pump of tank water control for user %d", from);
        }
    } else if (!strcmp(message, "Набор Воды Отключить")) {
        if (!RpcTankValveSet(unit->id, tank_name, false)) {
            g_string_append_printf(text, "        <b>Ошибка отключения Набора воды Бака</b>\n");
            LogF(LOG_TYPE_ERROR, "TGTANK", "Failed to disable pump of tank water control for user %d", from);
        }
    } else if (!strcmp(message, "Набор Воды Включить")) {
        if (!RpcTankValveSet(unit->id, tank_name, true)) {
            g_string_append_printf(text, "        <b>Ошибка включения Набора воды Бака</b>\n");
            LogF(LOG_TYPE_ERROR, "TGTANK", "Failed to enable pump of tank water control for user %d", from);
        }
    }

    if (RpcTanksGet(unit->id, &tanks)) {
        for (GList *s = tanks; s != NULL; s = s->next) {
            RpcTank *tank = (RpcTank *)s->data;

            if (!strcmp(tank->name, tank_name)) {
                g_string_append_printf(text, "<b>БАК: %s: %s</b>\n", unit->name, tank->name);

                if (tank->status) {
                    TgRespButtonAdd(buttons, "Отключить");
                    g_string_append_printf(text, "        Статус: <b>Включен</b>\n");
                } else {
                    TgRespButtonAdd(buttons, "Включить");
                    g_string_append_printf(text, "        Статус: <b>Отключен</b>\n");
                }

                g_string_append_printf(text, "        Уровень: <b>%u%%</b>\n", tank->level);

                if (tank->pump) {
                    TgRespButtonAdd(buttons, "Насос Отключить");
                    g_string_append_printf(text, "        Насос: <b>Работает</b>\n");
                } else {
                    TgRespButtonAdd(buttons, "Насос Включить");
                    g_string_append_printf(text, "        Насос: <b>Отключен</b>\n");
                }

                if (tank->valve) {
                    TgRespButtonAdd(buttons, "Набор Воды Отключить");
                    g_string_append_printf(text, "        Набор воды: <b>Работает</b>\n");
                } else {
                    TgRespButtonAdd(buttons, "Набор Воды Включить");
                    g_string_append_printf(text, "        Набор воды: <b>Отключен</b>\n");
                }
            }

            free(tank);
        }
        g_list_free(tanks);
        tanks = NULL;
    } else {
        text = g_string_append(text, "Ошибка получения списка баков");
        LogF(LOG_TYPE_ERROR, "TGTANK", "Failed to get tanks list for user %d", from);
    }

    TgRespButtonsAdd(buttons, 2, line_last);
    TgRespSend(token, from, text->str, buttons);
    g_string_free(text, true);
}
