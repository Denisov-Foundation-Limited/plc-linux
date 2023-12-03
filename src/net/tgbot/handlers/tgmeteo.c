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
#include <net/tgbot/handlers/tgmeteo.h>
#include <net/tgbot/tgresp.h>
#include <net/tgbot/tgmenu.h>
#include <utils/log.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void TgMeteoProcess(const char *token, unsigned from, const char *message)
{
    GList       *sensors = NULL;
    GString     *text = g_string_new("<b>МЕТЕО</b>\n\n");
    json_t      *buttons = json_array();
    GList       *units = NULL;
    const char  *line_last[] = {"Обновить", "Назад"};

    StackActiveUnitsGet(&units);

    for (GList *u = units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;

        if (RpcMeteoSensorsGet(unit->id, &sensors)) {
            if (g_list_length(sensors) > 0) {
                g_string_append_printf(text, "<b>%s:</b>\n", unit->name);
            }

            for (GList *s = sensors; s != NULL; s = s->next) {
                RpcMeteoSensor *sensor = (RpcMeteoSensor *)s->data;

                switch (sensor->type) {
                    case RPC_METEO_SENSOR_DS18B20:
                        g_string_append_printf(text, "        %s: <b>%.1f°</b>\n", sensor->name, sensor->ds18b20.temp);
                        break;
                }

                free(sensor);
            }
            g_list_free(sensors);
            sensors = NULL;
        } else {
            g_string_append_printf(text, "        <b>Ошибка</b>\n");
            LogF(LOG_TYPE_ERROR, "TGMETEO", "Failed to get meteo sensors for user \"%d\" and unit \"%s\"", from, unit->name);
        }
    }

    TgRespButtonsAdd(buttons, 2, line_last);
    TgRespSend(token, from, text->str, buttons);
    g_string_free(text, true);
}
