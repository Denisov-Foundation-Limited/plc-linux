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

typedef enum {
    SOCKET_GROUP_LIGHT,
    SOCKET_GROUP_SOCKET
} SocketGroup;

typedef struct {
    char        name[SHORT_STR_LEN];
    GpioPin     *gpio[SOCKET_PIN_MAX];
    SocketGroup group;
    bool        status;
} Socket;

/**
 * @brief Make new Socket struct
 * 
 * @param name Name of socket
 * @param button Socket button gpio
 * @param relay Socket relay gpio
 * @param group Socket group
 * 
 * @return Socket struct
 */
Socket *SocketNew(const char *name, GpioPin *button, GpioPin *relay, SocketGroup group);

/**
 * @brief Start socket buttons monitoring
 * 
 * @return True/False as result of starting monitoring
 */
bool SocketControllerStart();

/**
 * @brief Add new socket
 * 
 * @param sock Socket struct pointer
 */
void SocketAdd(Socket *sock);

/**
 * @brief Get all existing sockets list
 * 
 * @return Sockets list
 */
GList **SocketsGet();

/**
 * @brief Get socket by name
 * 
 * @param name Socket name
 * 
 * @return Socket Socket struct
 */
Socket *SocketGet(const char *name);

/**
 * @brief Set status of socket
 * 
 * @param sock Socket struct pointer
 * @param status New socket status
 * @param save Save to database status flag
 * 
 * @return True/False as result of setting status
 */
bool SocketStatusSet(Socket *sock, bool status, bool save);

/**
 * @brief Get current socket status
 * 
 * @param sock Socket struct pointer
 * 
 * @return True/False as socket status
 */
bool SocketStatusGet(Socket *sock);

#endif /* __SOCKET_CTRL_H__ */
