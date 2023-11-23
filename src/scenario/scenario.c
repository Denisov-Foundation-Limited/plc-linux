/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <scenario/scenario.h>
#include <utils/log.h>
#include <stack/rpc.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

GList *scenarios = NULL;

/*********************************************************************/
/*                                                                   */
/*                         PUBLIC  FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void ScenarioAdd(Scenario *scenario)
{
    scenarios = g_list_append(scenarios, (void *)scenario);
}

bool ScenarioStart(ScenarioType type)
{
    for (GList *s = scenarios; s != NULL; s = s->next) {
        Scenario *scenario = (Scenario *)s->data;

        if (scenario->type == type) {
            if (scenario->ctrl == SECURITY_CTRL_SOCKET) {
                if (!RpcSocketStatusSet(scenario->unit, scenario->socket.name, scenario->socket.status)) {
                    LogF(LOG_TYPE_ERROR, "SCENARIO", "Failed to switch socket \"%s\" status for unit \"%u\"", scenario->socket.name, scenario->unit);
                }
            }
        }
    }

    return true;
}
