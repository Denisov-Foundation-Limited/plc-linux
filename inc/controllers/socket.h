/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __SOCKET_CTRL_H__
#define __SOCKET_CTRL_H__

#include <stdbool.h>

#include <glib-2.0/glib.h>

#include <utils/utils.h>
#include <core/gpio.h>

#define SOCKET_DB_FILE  "socket.db"

typedef enum {
    SOCKET_PIN_BUTTON,
    SOCKET_PIN_RELAY,
    SOCKET_PIN_MAX
} SocketPin;

typedef struct {
    char    name[SHORT_STR_LEN];
    GpioPin *gpio[SOCKET_PIN_MAX];
    bool    status;
} Socket;

Socket *SocketNew(const char *name, GpioPin *button, GpioPin *relay);

bool SocketControllerStart();

void SocketAdd(Socket *sock);

GList **SocketsGet();

Socket *SocketGet(const char *name);

bool SocketStatusSet(Socket *sock, bool status, bool save);

bool SocketStatusGet(Socket *sock);

#endif /* __SOCKET_CTRL_H__ */
