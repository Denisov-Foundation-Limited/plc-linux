/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <db/database.h>
#include <utils/utils.h>

#include <stddef.h>
#include <string.h>
#include <stdio.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static char db_path[STR_LEN] = {0};

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void DatabasePathSet(const char *path)
{
    strncpy(db_path, path, STR_LEN);
}

bool DatabaseOpen(Database *db, const char *file_name)
{
    char    full_path[EXT_STR_LEN];

    snprintf(full_path, EXT_STR_LEN, "%s%s", db_path, file_name);

    if (sqlite3_open(full_path, &db->base)) {
        return false;
    }

    return true;
}

void DatabaseClose(Database *db)
{
    if (db->base != NULL) {
        sqlite3_close(db->base);
    }
}
