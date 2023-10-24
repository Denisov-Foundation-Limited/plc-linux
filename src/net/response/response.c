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

#include <net/response/response.h>
#include <utils/utils.h>
#include <utils/log.h>

bool ResponseFailSend(FCGX_Request *req, const char *module, const char *error)
{
    json_t *root = json_object();
    
    FCGX_PutS("Content-type: application/json\r\n", req->out);
    FCGX_PutS("HTTP/1.0 403 Forbidden\r\n", req->out);
    json_object_set_new(root, "error", json_string(error));
    json_object_set_new(root, "result", json_boolean(false));
    FCGX_PutS("\r\n", req->out);

    char *out = json_dumps(root, JSON_INDENT(4));
    FCGX_PutS(out, req->out);

    free(out);
    json_decref(root);

    Log(LOG_TYPE_ERROR, module, error);

    return false;
}

bool ResponseOkSend(FCGX_Request *req, json_t *root)
{
    FCGX_PutS("Content-type: application/json\r\n", req->out);
    FCGX_PutS("HTTP/1.0 200 OK\r\n", req->out);
    json_object_set_new(root, "result", json_boolean(true));
    FCGX_PutS("\r\n", req->out);

    char *out = json_dumps(root, JSON_INDENT(4));
    FCGX_PutS(out, req->out);

    free(out);
    json_decref(root);

    return true;
}
