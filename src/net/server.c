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
#include <curl/curl.h>

#include <net/server.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool Process(int socketId)
{
    FCGX_Request req;
    char query[255];

    if(FCGX_InitRequest(&req, socketId, 0) != 0) {
        printf("Failed to init request\n");
        return false;
    }

    for (;;) {
        if (FCGX_Accept_r(&req) < 0) {
            printf("failed to accept request\n");
            return false;
        }

        printf("ACCEPT NEW REQ\n");

        char *text = FCGX_GetParam("SCRIPT_NAME", req.envp);
        printf("NAME: %s\n", text);
        text = FCGX_GetParam("CONTENT_TYPE", req.envp);
        printf("CONTENT: %s\n", text);
        text = FCGX_GetParam("CONTENT_LENGTH", req.envp);
        printf("CONTENT LEN: %s\n", text);
        text = FCGX_GetParam("REQUEST_URI", req.envp);
        printf("REQUEST_URI: %s\n", text);
        text = FCGX_GetParam("REQUEST_METHOD", req.envp);
        printf("REQUEST_METHOD: %s\n", text);
       
        if (!strcmp(text, "POST")) {
            FCGX_GetStr(query, 255, req.in);
            printf("BODY: %s\n", query);
        }

        FCGX_PutS("Content-type: text/html\r\n", req.out);
        FCGX_PutS("\r\n", req.out);
        FCGX_PutS("<html><h1>HALLO</h1></html>\r\n", req.out);
        FCGX_Finish_r(&req);
    }
    return true;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool WebServerStart(const char *host, unsigned port)
{
    int socketId = 0;

    printf("Starting server\n");
    FCGX_Init();
    socketId = FCGX_OpenSocket("127.0.0.1:9090", 20);
    if (socketId < 0) {
        printf("Failed to start socket\n");
        return false;
    }
    Process(socketId);
    return true;
}
