/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <controllers/socket.h>
#include <utils/log.h>
#include <db/database.h>

#include <stdlib.h>
#include <threads.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct _Sockets {
    GList   *sockets;
    mtx_t   db_mtx;
} Sockets = {
    .sockets = NULL
};

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool StatusSave(const char *name, bool status)
{
    Database    db;
    char        sql[STR_LEN];
    char        con[STR_LEN];

    mtx_lock(&Sockets.db_mtx);

    if (!DatabaseOpen(&db, SOCKET_DB_FILE)) {
        DatabaseClose(&db);
        mtx_unlock(&Sockets.db_mtx);
        Log(LOG_TYPE_ERROR, "SOCKET", "Failed to load Socket database");
        return false;
    }

    snprintf(sql, STR_LEN, "status=%d", (int)status);
    snprintf(con, STR_LEN, "name=\"%s\"", name);

    if (!DatabaseUpdate(&db, "socket", sql, con)) {
        DatabaseClose(&db);
        mtx_unlock(&Sockets.db_mtx);
        Log(LOG_TYPE_ERROR, "SOCKET", "Failed to update Socket database");
        return false;
    }

    DatabaseClose(&db);
    mtx_unlock(&Sockets.db_mtx);
    return true;
}

static int SocketThread(void *data)
{
     for (;;) {
        bool pressed = false;

        for (GList *s = Sockets.sockets; s != NULL; s = s->next) {
            Socket *socket = (Socket *)s->data;

            if (GpioPinRead(socket->gpio[SOCKET_PIN_BUTTON])) {
                pressed = true;
                SocketStatusSet(socket, !SocketStatusGet(socket), true);
            }
        }

        if (pressed) {
            UtilsMsecSleep(800);
        }

        UtilsMsecSleep(200);
     }
}

/*********************************************************************/
/*                                                                   */
/*                         PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

Socket *SocketNew(const char *name, GpioPin *button, GpioPin *relay)
{
    Socket *socket = (Socket *)malloc(sizeof(Socket));

    strncpy(socket->name, name, SHORT_STR_LEN);
    socket->gpio[SOCKET_PIN_BUTTON] = button;
    socket->gpio[SOCKET_PIN_RELAY] = relay;

    return socket;
}

bool SocketControllerStart()
{
    thrd_t  sock_th;

    Log(LOG_TYPE_INFO, "SOCKET", "Starting Socket controller");

    thrd_create(&sock_th, &SocketThread, NULL);
    return true;
}

void SocketAdd(Socket *sock)
{
    Sockets.sockets = g_list_append(Sockets.sockets, sock);
}

GList **SocketsGet()
{
    return &Sockets.sockets;
}

Socket *SocketGet(const char *name)
{
    for (GList *s = Sockets.sockets; s != NULL; s = s->next) {
        Socket *socket = (Socket *)s->data;
        if (!strcmp(socket->name, name)) {
            return socket;
        }
    }
    return NULL;
}

bool SocketStatusSet(Socket *sock, bool status, bool save)
{
    sock->status = status;

    GpioPinWrite(sock->gpio[SOCKET_PIN_RELAY], status);

    if (status) {
        LogF(LOG_TYPE_INFO, "SOCKET", "Socket \"%s\" on", sock->name);
    } else {
        LogF(LOG_TYPE_INFO, "SOCKET", "Socket \"%s\" off", sock->name);
    }

    if (save) {
        if (!StatusSave(sock->name, status)) {
            return false;
        }
    }

    return true;
}

bool SocketStatusGet(Socket *sock)
{
    return sock->status;
}
