/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <glib-2.0/glib.h>

#include <db/dbloader.h>
#include <db/database.h>
#include <utils/utils.h>
#include <utils/log.h>
#include <controllers/security.h>
#include <controllers/socket.h>
#include <controllers/tank.h>

/*********************************************************************/
/*                                                                   */
/*                          PRIVATE FUNCTIONS                        */
/*                                                                   */
/*********************************************************************/

static bool DatabaseSecurityLoad()
{
    int         status = 0, alarm = 0;
    bool        exists = false;
    Database    db;
    char        sql[STR_LEN];

    if (!DatabaseOpen(&db, SECURITY_DB_FILE)) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to load Security database");
        return false;
    }

    if (!DatabaseCreate(&db, "security", "id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, status INTEGER, alarm INTEGER")) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to create Security table");
        return false;
    }

    status = 0;
    alarm = 0;

    if (!DatabaseRowExists(&db, "security", "name=\"controller\"", &exists)) {
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to check Security controller status");
        DatabaseClose(&db);
        return false;
    }

    if (exists) {
        snprintf(sql, STR_LEN, "name=\"controller\"");

        if (DatabaseFindOne(&db, "security", "status", sql, DATABASE_COL_TYPE_INT, (void *)&status)) {
            LogF(LOG_TYPE_INFO, "DBLOADER", "Loaded status for Security controller is \"%d\"", status);
        } else {
            Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to find Security controller status");
            DatabaseClose(&db);
            return false;
        }

        if (DatabaseFindOne(&db, "security", "alarm", sql, DATABASE_COL_TYPE_INT, (void *)&alarm)) {
            LogF(LOG_TYPE_INFO, "DBLOADER", "Loaded alarm status for Security controller is \"%d\"", status);
        } else {
            Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to find Security controller alarm status");
            DatabaseClose(&db);
            return false;
        }
    } else {
        snprintf(sql, STR_LEN, "\"controller\", %d, %d", 0, 0);

        if (DatabaseInsert(&db, "security", "name, status, alarm", sql)) {
            LogF(LOG_TYPE_INFO, "DBLOADER", "Created status for Security controller is \"%d\"", status);
        } else {
            Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to insert Security controller status");
            DatabaseClose(&db);
            return false;
        }
    }

    if (!SecurityStatusSet((bool)status, false)) {
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to load Security controller status");
        DatabaseClose(&db);
        return false;
    }

    if ((bool)alarm) {
        if (!SecurityAlarmSet((bool)alarm, false)) {
            Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to load Security controller alarm status");
            DatabaseClose(&db);
            return false;
        }
    }

    DatabaseClose(&db);

    return true;
}

static bool DatabaseSocketLoad()
{
    int         status = 0;
    bool        exists = false;
    Database    db;
    char        sql[STR_LEN];

    if (!DatabaseOpen(&db, SOCKET_DB_FILE)) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to load Socket database");
        return false;
    }

    if (!DatabaseCreate(&db, "socket", "id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, status INTEGER")) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to create Socket table");
        return false;
    }

    for (GList *s = *SocketsGet(); s != NULL; s = s->next) {
        Socket *socket = (Socket *)s->data;
        status = 0;

        snprintf(sql, STR_LEN, "name=\"%s\"", socket->name);

        if (!DatabaseRowExists(&db, "socket", sql, &exists)) {
            LogF(LOG_TYPE_ERROR, "DBLOADER", "Failed to check Socket \"%s\" status", socket->name);
            DatabaseClose(&db);
            return false;
        }

        if (exists) {
            snprintf(sql, STR_LEN, "name=\"%s\"", socket->name);

            if (DatabaseFindOne(&db, "socket", "status", sql, DATABASE_COL_TYPE_INT, (void *)&status)) {
                LogF(LOG_TYPE_INFO, "DBLOADER", "Loaded status for Socket \"%s\" is \"%d\"", socket->name, status);
            } else {
                LogF(LOG_TYPE_ERROR, "DBLOADER", "Failed to find Socket \"%s\" status", socket->name);
                DatabaseClose(&db);
                return false;
            }
        } else {
            snprintf(sql, STR_LEN, "\"%s\", %d", socket->name, 0);
        
            if (DatabaseInsert(&db, "socket", "name, status", sql)) {
                LogF(LOG_TYPE_INFO, "DBLOADER", "Created status for Socket controller is \"%d\"", status);
            } else {
                Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to insert Socket controller status");
                DatabaseClose(&db);
                return false;
            }
        }

        if (!SocketStatusSet(socket, (bool)status, false)) {
            LogF(LOG_TYPE_ERROR, "DBLOADER", "Failed to set Socket \"%s\" status", socket->name);
            DatabaseClose(&db);
            return false;
        }
    }

    DatabaseClose(&db);

    return true;
}

static bool DatabaseTankLoad()
{
    int         status = 0;
    bool        exists = false;
    Database    db;
    char        sql[STR_LEN];

    if (!DatabaseOpen(&db, TANK_DB_FILE)) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to load Tank database");
        return false;
    }

    if (!DatabaseCreate(&db, "tank", "id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, status INTEGER")) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to create Tank table");
        return false;
    }

    for (GList *t = *TanksGet(); t != NULL; t = t->next) {
        Tank *tank = (Tank *)t->data;
        status = 0;

        snprintf(sql, STR_LEN, "name=\"%s\"", tank->name);

        if (!DatabaseRowExists(&db, "tank", sql, &exists)) {
            LogF(LOG_TYPE_ERROR, "DBLOADER", "Failed to check Tank \"%s\" status", tank->name);
            DatabaseClose(&db);
            return false;
        }

        if (exists) {
            snprintf(sql, STR_LEN, "name=\"%s\"", tank->name);

            if (DatabaseFindOne(&db, "tank", "status", sql, DATABASE_COL_TYPE_INT, (void *)&status)) {
                LogF(LOG_TYPE_INFO, "DBLOADER", "Loaded status for Tank \"%s\" is \"%d\"", tank->name, status);
            } else {
                LogF(LOG_TYPE_ERROR, "DBLOADER", "Failed to find Tank \"%s\" status", tank->name);
                DatabaseClose(&db);
                return false;
            }
        } else {
            snprintf(sql, STR_LEN, "\"%s\", %d", tank->name, 0);

            if (DatabaseInsert(&db, "tank", "name, status", sql)) {
                LogF(LOG_TYPE_INFO, "DBLOADER", "Created status for Tank controller is \"%d\"", status);
            } else {
                Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to insert Tank controller status");
                DatabaseClose(&db);
                return false;
            }
        }

        if (!TankStatusSet(tank, (bool)status, false)) {
            LogF(LOG_TYPE_ERROR, "DBLOADER", "Failed to set Tank \"%s\" status", tank->name);
            DatabaseClose(&db);
            return false;
        }
    }

    DatabaseClose(&db);

    return true;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool DatabaseLoaderLoad()
{
    if (!DatabaseSecurityLoad()) {
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to load security states from DB");
        return false;
    }

    if (!DatabaseSocketLoad()) {
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to load socket states from DB");
        return false;
    }

    if (!DatabaseTankLoad()) {
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to load tank states from DB");
        return false;
    }

    return true;
}
