/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <fcgi_config.h>
#include <fcgiapp.h>
#include <utils/utils.h>
#include <utils/log.h>
#include <net/web/webserver.h>

#include <net/web/handlers/securityh.h>
#include <net/web/handlers/socketh.h>
#include <net/web/handlers/meteoh.h>
#include <net/web/handlers/indexh.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct {
    char        ip[STR_LEN];
    unsigned    port;
} Server = {
    .ip = {0},
    .port = 0
};

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool Process(int socketId)
{
    FCGX_Request    req;
    GList           *params = NULL;

    if (FCGX_InitRequest(&req, socketId, 0) != 0) {
        Log(LOG_TYPE_ERROR, "SERVER", "Failed to init web request");
        return false;
    }

    for (;;) {
        if (FCGX_Accept_r(&req) < 0) {
            Log(LOG_TYPE_ERROR, "SERVER", "Failed to accept request");
            continue;
        }

        const char *query = FCGX_GetParam("SCRIPT_NAME", req.envp);
        char *url = FCGX_GetParam("REQUEST_URI", req.envp);

        if (!strcmp(query, "/")) {
            if (!HandlerIndexProcess(&req, NULL)) {
                Log(LOG_TYPE_ERROR, "SERVER", "Failed to process Index handler");
            }
            FCGX_Finish_r(&req);
            continue;
        } 

        if (UtilsURIParse(url, &params)) {
            if (!strcmp(query, "/api/" SERVER_API_VER "/security")) {
                if (!HandlerSecurityProcess(&req, &params)) {
                    Log(LOG_TYPE_ERROR, "SERVER", "Failed to process Security controller get handler");
                }
            } else if (!strcmp(query, "/api/" SERVER_API_VER "/meteo")) {
                if (!HandlerMeteoProcess(&req, &params)) {
                    Log(LOG_TYPE_ERROR, "SERVER", "Failed to process Meteo controller get handler");
                }
            } else if (!strcmp(query, "/api/" SERVER_API_VER "/socket")) {
                if (!HandlerSocketProcess(&req, &params)) {
                    Log(LOG_TYPE_ERROR, "SERVER", "Failed to process Socket controller get handler");
                }
            } else {
                FCGX_PutS("Content-type: text/html\r\n", req.out);
                FCGX_PutS("\r\n", req.out);
                FCGX_PutS("<html><h1>404 NOT FOUND</h1></html>\r\n", req.out);
            }
        } else {
            Log(LOG_TYPE_ERROR, "SERVER", "Incorrect request");
        }

        if (params != NULL) {
            for (GList *p = params; p != NULL; p = p->next) {
                UtilsReqParam *param = (UtilsReqParam *)p->data;
                free(param);
            }
            g_list_free(params);
            params = NULL;
        }

        FCGX_Finish_r(&req);
    }
    return true;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void WebServerCredsSet(const char *host, unsigned port)
{
    strncpy(Server.ip, host, STR_LEN);
    Server.port = port;
}

bool WebServerStart()
{
    /*thrd_t  th;
    int     res;

    thrd_create(th, &WebServerThread, NULL);
    thrd_join(th, &res);

    if (res < 0) {
        return false;
    }*/

    int     socketId = 0;
    char    full_path[EXT_STR_LEN];

    snprintf(full_path, EXT_STR_LEN, "%s:%d", Server.ip, Server.port);

    FCGX_Init();
    socketId = FCGX_OpenSocket(full_path, 20);
    if (socketId < 0) {
        return false;
    }

    Process(socketId);

    return true;
}
