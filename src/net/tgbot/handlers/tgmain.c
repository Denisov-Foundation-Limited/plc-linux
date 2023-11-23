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

#include <net/tgbot/handlers/tgmain.h>
#include <net/tgbot/tgresp.h>
#include <stack/rpc.h>
#include <utils/log.h>
#include <scenario/scenario.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void TgMainProcess(const char *token, unsigned from, const char *message)
{
    json_t      *buttons = json_array();
    const char  *line0[] = { "Я дома", "Ушёл" };
    const char  *line1[] = { "Камеры", "Метео", "Охрана", "Свет" };
    const char  *line2[] = { "Розетки", "Термо", "Бак", "Полив" };

    if (!strcmp(message, "Я дома")) {
        if (!RpcSecurityStatusSet(RPC_DEFAULT_UNIT, false)) {
            Log(LOG_TYPE_ERROR, "TGMAIN", "Failed to set security status");
        }
        if (!ScenarioStart(SCENARIO_IN_HOME)) {
            Log(LOG_TYPE_ERROR, "TGMAIN", "Failed to start scenario IN_HOME");
        }
    } else if (!strcmp(message, "Ушёл")) {
        if (!RpcSecurityStatusSet(RPC_DEFAULT_UNIT, true)) {
            Log(LOG_TYPE_ERROR, "TGMAIN", "Failed to set security status");
        }
        if (!ScenarioStart(SCENARIO_OUT_HOME)) {
            Log(LOG_TYPE_ERROR, "TGMAIN", "Failed to start scenario OUT_HOME");
        }
    }

    TgRespButtonsAdd(buttons, 2, line0);
    TgRespButtonsAdd(buttons, 4, line1);
    TgRespButtonsAdd(buttons, 4, line2);
    TgRespSend(token, from, "<b>ГЛАВНОЕ МЕНЮ</b>\n", buttons);
}
