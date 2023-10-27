/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <glib.h>

#include <db/dbloader.h>
#include <db/database.h>
#include <utils/utils.h>
#include <utils/log.h>
#include <controllers/security.h>

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

    if (!DatabaseCreate(&db, "controllers", "id INTEGER PRIMARY KEY AUTOINCREMENT, name TEXT, status INTEGER, alarm INTEGER")) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to create Security table");
        return false;
    }

    for (GList *c = *SecurityControllersGet(); c != NULL; c = c->next) {
        SecurityController *ctrl = (SecurityController *)c->data;
        status = 0;
        alarm = 0;

        snprintf(sql, STR_LEN, "name=\"%s\"", ctrl->name);
        if (!DatabaseExists(&db, "controllers", sql, &exists)) {
            Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to check Security controller status");
            break;
        }

        if (exists) {
            snprintf(sql, STR_LEN, "name=\"%s\"", ctrl->name);

            if (DatabaseFindOne(&db, "controllers", "status", sql, DATABASE_COL_TYPE_INT, (void *)&status)) {
                LogF(LOG_TYPE_INFO, "DBLOADER", "Loaded status for Security controller \"%s\" is \"%d\"", ctrl->name, status);
            } else {
                Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to find Security controller status");
                break;
            }

            if (DatabaseFindOne(&db, "controllers", "alarm", sql, DATABASE_COL_TYPE_INT, (void *)&alarm)) {
                LogF(LOG_TYPE_INFO, "DBLOADER", "Loaded alarm status for Security controller \"%s\" is \"%d\"", ctrl->name, status);
            } else {
                Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to find Security controller alarm status");
                break;
            }
        } else {
            snprintf(sql, STR_LEN, "\"%s\", %d, %d", ctrl->name, 0, 0);
            
            if (DatabaseInsert(&db, "controllers", "name, status, alarm", sql)) {
                LogF(LOG_TYPE_INFO, "DBLOADER", "Created status for Security controller \"%s\" is \"%d\"", ctrl->name, status);
            } else {
                Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to insert Security controller status");
                break;
            }
        }

        if (!SecurityStatusSet(ctrl, (bool)status, false)) {
            Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to load Security controller status");
            break;
        }

        if ((bool)status) {
            if (!SecurityAlarmSet(ctrl, (bool)alarm, false)) {
                Log(LOG_TYPE_ERROR, "DBLOADER", "Failed to load Security controller alarm status");
                break;
            }
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
    return true;
}
