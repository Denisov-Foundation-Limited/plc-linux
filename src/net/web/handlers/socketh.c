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

#include <net/web/handlers/socketh.h>
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
        return ResponseFailSend(req, "SOCKETH", "Socket command ivalid");
    }

    if (!RpcSocketStatusSet(RPC_DEFAULT_UNIT, name, status)) {
        return ResponseFailSend(req, "SOCKETH", "Failed to set socket status");
    }

    return ResponseOkSend(req, root);
}

static bool HandlerSocketsGet(FCGX_Request *req, GList **params)
{
    json_t  *root = json_object();
    GList   *sockets = NULL;

    if (!RpcSocketsGet(RPC_DEFAULT_UNIT, &sockets)) {
        return ResponseFailSend(req, "SOCKETH", "Failed to get meteo sockets");
    }

    json_t *jsockets = json_array();

    for (GList *s = sockets; s != NULL; s = s->next) {
        RpcSocket *socket = (RpcSocket *)s->data;

        json_t *jsocket = json_object();
        json_object_set_new(jsocket, "name", json_string(socket->name));
        json_object_set_new(jsocket, "status", json_boolean(socket->status));
        json_array_append_new(jsockets, jsocket);

        switch (socket->group) {
            case RPC_SOCKET_GROUP_LIGHT:
                json_object_set_new(jsocket, "group", json_string("light"));
                break;

            case RPC_SOCKET_GROUP_SOCKET:
                json_object_set_new(jsocket, "group", json_string("socket"));
                break;
        }

        free(socket);
    }

    json_object_set_new(root, "sockets", jsockets);
    g_list_free(sockets);

    return ResponseOkSend(req, root);
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool HandlerSocketProcess(FCGX_Request *req, GList **params)
{
    for (GList *p = *params; p != NULL; p = p->next) {
        UtilsReqParam *param = (UtilsReqParam *)p->data;

        if (!strcmp(param->name, "cmd")) {
            if (!strcmp(param->value, "status_set")) {
                return HandlerStatusSet(req, params);
            } else if (!strcmp(param->value, "sockets_get")) {
                return HandlerSocketsGet(req, params);
            } else {
                return false;
            }
        }
    }

    return true;
}
