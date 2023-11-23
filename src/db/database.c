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

bool DatabaseExec(Database *db, const char *sql)
{
    int ret;

    ret = sqlite3_exec(db->base, sql, NULL, NULL, NULL);
    if(ret != SQLITE_OK) {
        return false;
    }

    return true;
}

bool DatabaseCreate(Database *db, const char *table, const char *sql)
{
    char    request[STR_LEN];

    snprintf(request, STR_LEN, "CREATE TABLE IF NOT EXISTS %s(%s);", table, sql);

    return DatabaseExec(db, request);
}

bool DatabaseDrop(Database *db, const char *table)
{
    char    request[STR_LEN];

    snprintf(request, STR_LEN, "DROP TABLE %s;", table);

    return DatabaseExec(db, request);
}

bool DatabaseInsert(Database *db, const char *table, const char *columns, const char *values)
{
    char    request[STR_LEN];

    snprintf(request, STR_LEN, "INSERT INTO %s (%s) VALUES (%s);", table, columns, values);

    return DatabaseExec(db, request);
}

bool DatabaseUpdate(Database *db, const char *table, const char *sql, const char *conditions)
{
    char    request[STR_LEN];

    snprintf(request, STR_LEN, "UPDATE %s SET %s WHERE %s;", table, sql, conditions);

    return DatabaseExec(db, request);
}

bool DatabaseRowExists(Database *db, const char *table, const char *sql, bool *exists)
{
    int             ret;
    sqlite3_stmt    *stmt;
    char            request[STR_LEN];

    *exists = false;
    snprintf(request, STR_LEN, "SELECT * FROM %s WHERE %s LIMIT 1;", table, sql);

    do {
        sqlite3_prepare(db->base, request, -1, &stmt, 0);
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            *exists = true;
        }
        ret = sqlite3_finalize(stmt);
    }
    while (ret == SQLITE_SCHEMA);

    return true;
}

bool DatabaseFindOne(Database *db, const char *table, const char *conditions, const char *sql, DatabaseColType type, void *data)
{
    char            request[STR_LEN];
    int             ret;
    sqlite3_stmt    *stmt;

    if (data == NULL) {
        return false;
    }

    snprintf(request, STR_LEN, "SELECT %s FROM %s WHERE %s LIMIT 1;", conditions, table, sql);

    do {
        sqlite3_prepare(db->base, request, -1, &stmt, 0);
        while (sqlite3_step(stmt) == SQLITE_ROW) {
            switch (type) {
                case DATABASE_COL_TYPE_STRING:
                    strncpy((char *)data, (const char *)sqlite3_column_text(stmt, 0), STR_LEN);
                    break;

                case DATABASE_COL_TYPE_INT:
                    *(int *)data = sqlite3_column_int(stmt, 0);
                    break;

                case DATABASE_COL_TYPE_DOUBLE:
                    *(double *)data = sqlite3_column_double(stmt, 0);
                    break;
            }
        }
        ret = sqlite3_finalize(stmt);
    }
    while (ret == SQLITE_SCHEMA);

    return true;
}

bool DatabaseFindAll(Database *db, const char *table, GList **columns, GList **out)
{
    char            request[STR_LEN];
    int             ret;
    sqlite3_stmt    *stmt;

    snprintf(request, STR_LEN, "SELECT * FROM %s;", table);

    do {
        sqlite3_prepare(db->base, request, -1, &stmt, 0);
        while(sqlite3_step(stmt) == SQLITE_ROW)
        {
            DatabaseRow *row = (DatabaseRow *)malloc(sizeof(DatabaseRow));
            row->value = NULL;

            for (GList *c = *columns; c != NULL; c = c->next) {
                DatabaseColumn *col = (DatabaseColumn *)c->data;

                DatabaseData *data = (DatabaseData *)malloc(sizeof(DatabaseData));
                switch (col->type) {
                    case DATABASE_COL_TYPE_STRING:
                        strncpy(data->text, (const char *)sqlite3_column_text(stmt, col->id), STR_LEN);
                        break;

                    case DATABASE_COL_TYPE_INT:
                        data->integer = sqlite3_column_int(stmt, col->id);
                        break;

                    case DATABASE_COL_TYPE_DOUBLE:
                        data->double_num = sqlite3_column_double(stmt, col->id);
                        break;
                }
                row->value = g_list_append(row->value, data);
            }

            *out = g_list_append(*out, row);
        }
        ret = sqlite3_finalize(stmt);
    }
    while (ret == SQLITE_SCHEMA);

    return true;
}

void DatabaseClose(Database *db)
{
    if (db->base != NULL) {
        sqlite3_close(db->base);
    }
}
