/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>

#include <glib-2.0/glib.h>
#include <jansson.h>
#include <fcgiapp.h>

#include <net/web/handlers/tankh.h>
#include <net/web/response.h>
#include <utils/utils.h>
#include <utils/log.h>
#include <stack/rpc.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool HandlerStatusSet(FCGX_Request *req, GList **params)
{
    json_t      *root = json_object();
    bool        found = false;
    bool        status = false;
    unsigned    unit = RPC_DEFAULT_UNIT;
    char        name[STR_LEN] = {0};

    for (GList *p = *params; p != NULL; p = p->next) {
        UtilsReqParam *param = (UtilsReqParam *)p->data;

        if (!strcmp(param->name, "status")) {
            if (!strcmp(param->value, "true")) {
                status = true;
                found = true;
            } else if (!strcmp(param->value, "false")) {
                found = true;
                status = false;
            }
        } else if (!strcmp(param->name, "name")) {
            strncpy(name, param->value, STR_LEN);
            found = true;
        } else if (!strcmp(param->name, "unit")) {
            unit = atoi(param->value);
            found = true;
        }
    }

    if (!found) {
        return ResponseFailSend(req, "TANKH", "Tank command ivalid");
    }

    if (!RpcTankStatusSet(unit, name, status)) {
        return ResponseFailSend(req, "TANKH", "Failed to set tank status");
    }

    return ResponseOkSend(req, root);
}

static bool HandlerPumpSet(FCGX_Request *req, GList **params)
{
    json_t      *root = json_object();
    bool        found = false;
    bool        status = false;
    unsigned    unit = RPC_DEFAULT_UNIT;
    char        name[STR_LEN] = {0};

    for (GList *p = *params; p != NULL; p = p->next) {
        UtilsReqParam *param = (UtilsReqParam *)p->data;

        if (!strcmp(param->name, "status")) {
            if (!strcmp(param->value, "true")) {
                status = true;
                found = true;
            } else if (!strcmp(param->value, "false")) {
                found = true;
                status = false;
            }
        } else if (!strcmp(param->name, "name")) {
            strncpy(name, param->value, STR_LEN);
            found = true;
        } else if (!strcmp(param->name, "unit")) {
            unit = atoi(param->value);
            found = true;
        }
    }

    if (!found) {
        return ResponseFailSend(req, "TANKH", "Tank command ivalid");
    }

    if (!RpcTankPumpSet(unit, name, status)) {
        return ResponseFailSend(req, "TANKH", "Failed to set tank pump status");
    }

    return ResponseOkSend(req, root);
}

static bool HandlerValveSet(FCGX_Request *req, GList **params)
{
    json_t  *root = json_object();
    bool    found = false;
    bool    status = false;
    char    name[STR_LEN] = {0};

    for (GList *p = *params; p != NULL; p = p->next) {
        UtilsReqParam *param = (UtilsReqParam *)p->data;

        if (!strcmp(param->name, "status")) {
            if (!strcmp(param->value, "true")) {
                status = true;
                found = true;
            } else if (!strcmp(param->value, "false")) {
                found = true;
                status = false;
            }
        } else if (!strcmp(param->name, "name")) {
            strncpy(name, param->value, STR_LEN);
            found = true;
        }
    }

    if (!found) {
        return ResponseFailSend(req, "TANKH", "Tank command ivalid");
    }

    if (!RpcTankValveSet(RPC_DEFAULT_UNIT, name, status)) {
        return ResponseFailSend(req, "TANKH", "Failed to set tank valve status");
    }

    return ResponseOkSend(req, root);
}

static bool HandlerTanksGet(FCGX_Request *req, GList **params)
{
    json_t  *root = json_object();
    GList   *tanks = NULL;

    if (!RpcTanksGet(RPC_DEFAULT_UNIT, &tanks)) {
        return ResponseFailSend(req, "TANKH", "Failed to get meteo tanks");
    }

    json_t *jtanks = json_array();

    for (GList *s = tanks; s != NULL; s = s->next) {
        RpcTank *tank = (RpcTank *)s->data;

        json_t *jtank = json_object();
        json_object_set_new(jtank, "name", json_string(tank->name));
        json_object_set_new(jtank, "status", json_boolean(tank->status));
        json_object_set_new(jtank, "pump", json_boolean(tank->pump));
        json_object_set_new(jtank, "valve", json_boolean(tank->valve));
        json_object_set_new(jtank, "level", json_integer(tank->level));
        json_array_append_new(jtanks, jtank);

        free(tank);
    }

    json_object_set_new(root, "tanks", jtanks);
    g_list_free(tanks);

    return ResponseOkSend(req, root);
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool HandlerTankProcess(FCGX_Request *req, GList **params)
{
    for (GList *p = *params; p != NULL; p = p->next) {
        UtilsReqParam *param = (UtilsReqParam *)p->data;

        if (!strcmp(param->name, "cmd")) {
            if (!strcmp(param->value, "status_set")) {
                return HandlerStatusSet(req, params);
            } else if (!strcmp(param->value, "tanks_get")) {
                return HandlerTanksGet(req, params);
            } else if (!strcmp(param->value, "pump_set")) {
                return HandlerPumpSet(req, params);
            } else if (!strcmp(param->value, "valve_set")) {
                return HandlerValveSet(req, params);
            } else {
                return false;
            }
        }
    }

    return true;
}
