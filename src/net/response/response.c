/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <net/response/response.h>

bool ResponseSend(ResponseType type, FCGX_Request *req, json_t *root)
{
    FCGX_PutS("Content-type: application/json\r\n", req->out);
    if (type == RESPONSE_TYPE_OK) {
        FCGX_PutS("HTTP/1.0 200 OK\r\n", req->out);
        json_object_set_new(root, "result", json_boolean(true));
    } else {
        FCGX_PutS("HTTP/1.0 403 Forbidden\r\n", req->out);
        json_object_set_new(root, "result", json_boolean(false));
    }
    FCGX_PutS("\r\n", req->out);

    char *out = json_dumps(root, JSON_INDENT(4));
    FCGX_PutS(out, req->out);

    free(out);
    json_decref(root);

    if (type == RESPONSE_TYPE_FAIL) {
        return false;
    }

    return true;
}
