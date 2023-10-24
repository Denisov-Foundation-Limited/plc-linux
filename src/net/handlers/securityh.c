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

#include <glib.h>
#include <jansson.h>
#include <fcgiapp.h>

#include <net/handlers/securityh.h>
#include <net/response/response.h>
#include <utils/utils.h>
#include <utils/log.h>
#include <controllers/security.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool HandlerStatusSet(FCGX_Request *req, GList **params)
{
    bool    status = false;
    char    ctrl_name[SHORT_STR_LEN] = {0};
    json_t  *root = json_object();
    char    error[STR_LEN];

    for (GList *p = *params; p != NULL; p = p->next) {
        UtilsReqParam *param = (UtilsReqParam *)p->data;

        if (!strcmp(param->name, "status")) {
            if (!strcmp(param->value, "true")) {
                status = true;
            }
        } else if (!strcmp(param->name, "ctrl")) {
            strncpy(ctrl_name, param->value, SHORT_STR_LEN);
        }
    }

    SecurityController *ctrl = SecurityControllerGet(ctrl_name);
    if (ctrl == NULL) {
        snprintf(error, STR_LEN, "Security controller \"%s\" not found", ctrl_name);
        json_object_set_new(root, "error", json_string(error));
        Log(LOG_TYPE_ERROR, "SECURITYH", error);
        return ResponseSend(RESPONSE_TYPE_FAIL, req, root);
    }

    if (!SecurityStatusSet(ctrl, status)) {
        snprintf(error, STR_LEN, "Security controller \"%s\" failed to switch status to \"%d\"", ctrl_name, status);
        json_object_set_new(root, "error", json_string(error));
        Log(LOG_TYPE_ERROR, "SECURITYH", error);
        return ResponseSend(RESPONSE_TYPE_FAIL, req, root);
    }

    return ResponseSend(RESPONSE_TYPE_OK, req, root);
}

static bool HandlerStatusGet(FCGX_Request *req, GList **params)
{
    char    status[SHORT_STR_LEN];
    char    ctrl_name[SHORT_STR_LEN] = {0};
    char    error[STR_LEN];
    json_t  *root = json_object();

    for (GList *p = *params; p != NULL; p = p->next) {
        UtilsReqParam *param = (UtilsReqParam *)p->data;

        if (!strcmp(param->name, "ctrl")) {
            strncpy(ctrl_name, param->value, SHORT_STR_LEN);
        }
    }

    SecurityController *ctrl = SecurityControllerGet(ctrl_name);
    if (ctrl == NULL) {
        snprintf(error, STR_LEN, "Security controller \"%s\" not found", ctrl_name);
        json_object_set_new(root, "error", json_string(error));
        LogF(LOG_TYPE_ERROR, "SECURITYH", error);
        return ResponseSend(RESPONSE_TYPE_FAIL, req, root);
    }

    json_object_set_new(root, "status", json_boolean(SecurityStatusGet(ctrl)));

    return ResponseSend(RESPONSE_TYPE_OK, req, root);
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool HandlerSecurity(FCGX_Request *req, GList **params)
{
    for (GList *p = *params; p != NULL; p = p->next) {
        UtilsReqParam *param = (UtilsReqParam *)p->data;

        if (!strcmp(param->name, "cmd")) {
            if (!strcmp(param->value, "status_set")) {
                return HandlerStatusSet(req, params);
            } else if (!strcmp(param->value, "status_get")) {
                return HandlerStatusGet(req, params);
            } else {
                return false;
            }
        }
    }

    return true;
}
