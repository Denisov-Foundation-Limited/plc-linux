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

#include <net/web/handlers/watererh.h>
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
        }
    }

    if (!found) {
        return ResponseFailSend(req, "WATERERH", "Waterer command ivalid");
    }

    if (!RpcWatererStatusSet(RPC_DEFAULT_UNIT, name, status)) {
        return ResponseFailSend(req, "WATERERH", "Failed to set waterer status");
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
        return ResponseFailSend(req, "WATERERH", "Waterer command ivalid");
    }

    if (!RpcWatererValveSet(RPC_DEFAULT_UNIT, name, status)) {
        return ResponseFailSend(req, "WATERERH", "Failed to set waterer valve status");
    }

    return ResponseOkSend(req, root);
}

static bool HandlerWaterersGet(FCGX_Request *req, GList **params)
{
    json_t  *root = json_object();
    GList   *waterers = NULL;

    if (!RpcWaterersGet(RPC_DEFAULT_UNIT, &waterers)) {
        return ResponseFailSend(req, "WATERERH", "Failed to get meteo waterers");
    }

    json_t *jwaterers = json_array();

    for (GList *s = waterers; s != NULL; s = s->next) {
        RpcWaterer *waterer = (RpcWaterer *)s->data;

        json_t *jwaterer = json_object();
        json_object_set_new(jwaterer, "name", json_string(waterer->name));
        json_object_set_new(jwaterer, "status", json_boolean(waterer->status));
        json_object_set_new(jwaterer, "valve", json_boolean(waterer->valve));

        json_t *jtimes = json_object();
        for (GList *t = waterer->times; t != NULL; t = t->next) {
            RpcWatererTime *tm = (RpcWatererTime *)t->data;

            json_t *jtime = json_object();
            json_object_set_new(jtime, "day", json_integer(tm->day));
            json_object_set_new(jtime, "hour", json_integer(tm->hour));
            json_object_set_new(jtime, "min", json_integer(tm->min));
            json_object_set_new(jtime, "state", json_integer(tm->state));
            json_array_append_new(jtimes, jtime);

            free(tm);
        }
        json_object_set_new(jwaterer, "times", jtimes);
    
        json_array_append_new(jwaterers, jwaterer);

        g_list_free(waterer->times);
        free(waterer);
    }

    json_object_set_new(root, "waterers", jwaterers);
    g_list_free(waterers);

    return ResponseOkSend(req, root);
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool HandlerWatererProcess(FCGX_Request *req, GList **params)
{
    for (GList *p = *params; p != NULL; p = p->next) {
        UtilsReqParam *param = (UtilsReqParam *)p->data;

        if (!strcmp(param->name, "cmd")) {
            if (!strcmp(param->value, "status_set")) {
                return HandlerStatusSet(req, params);
            } else if (!strcmp(param->value, "waterers_get")) {
                return HandlerWaterersGet(req, params);
            } else if (!strcmp(param->value, "valve_set")) {
                return HandlerValveSet(req, params);
            } else {
                return false;
            }
        }
    }

    return true;
}
