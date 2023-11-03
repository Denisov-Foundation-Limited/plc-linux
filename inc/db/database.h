/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __DATABASE_H__
#define __DATABASE_H__

#include <stdbool.h>

#include <glib-2.0/glib.h>
#include <sqlite3.h>

#include <utils/utils.h>

typedef enum {
    DATABASE_COL_TYPE_STRING,
    DATABASE_COL_TYPE_INT,
    DATABASE_COL_TYPE_DOUBLE
} DatabaseColType;

typedef struct {
    sqlite3 *base;
} Database;

typedef struct {
    unsigned        id;
    DatabaseColType type;
} DatabaseColumn;

typedef struct {
    int     integer;
    double  double_num;
    char    text[STR_LEN];
} DatabaseData;

typedef struct {
    GList   *value;
} DatabaseRow;

/**
 * @brief Set path for database files
 *
 * @param path Path to DB files
 */
void DatabasePathSet(const char *path);

/**
 * @brief Load sqlite3 database from file
 *
 * @param db Database storage
 * @param file_name Path to database
 *
 * @return True/false as result of db load
 */
bool DatabaseOpen(Database *db, const char *file_name);

/**
 * @brief Create SQL table
 *
 * @param db Database storage
 * @param table Database table name
 * @param sql SQL request
 *
 * @return True/false as result
 */
bool DatabaseCreate(Database *db, const char *table, const char *sql);

/**
 * @brief Check record exixts in table
 *
 * @param db Database storage
 * @param table Database table
 * @param sql SQL check conditions
 * @param exists Exists result
 *
 * @return True/false as result
 */
bool DatabaseRowExists(Database *db, const char *table, const char *sql, bool *exists);

/**
 * @brief Exec raw SQL request
 *
 * @param db Database storage
 * @param sql SQL request
 *
 * @return True/false as result
 */
bool DatabaseExec(Database *db, const char *sql);

/**
 * @brief Delete SQL table
 *
 * @param db Database storage
 * @param table Database table name
 *
 * @return True/false as result
 */
bool DatabaseDrop(Database *db, const char *table);

/**
 * @brief Insert values in SQL table
 *
 * @param db Database storage
 * @param table Database table name
 * @param columns Table columns names list
 * @param values Output values
 *
 * @return True/false as result
 */
bool DatabaseInsert(Database *db, const char *table, const char *columns, const char *values);

/**
 * @brief Check record exists in SQL table
 *
 * @param db Database storage
 * @param table Database table name
 * @param sql SQL request
 * @param conditions Update request conditions
 *
 * @return True/false as result
 */
bool DatabaseUpdate(Database *db, const char *table, const char *sql, const char *conditions);

/**
 * @brief Get data from SQL table
 *
 * @param db Database storage
 * @param table Database table name
 * @param conditions Search conditions
 * @param sql SQL search request
 * @param type Out data type
 * @param data Out value
 *
 * @return True/false as result
 */
bool DatabaseFindOne(Database *db, const char *table, const char *conditions, const char *sql, DatabaseColType type, void *data);

/**
 * @brief Get all data from SQL table
 *
 * @param db Database storage
 * @param table Database table name
 * @param columns Table columns names list
 * @param out Output data
 *
 * @return True/false as result
 */
bool DatabaseFindAll(Database *db, const char *table, GList **columns, GList **out);

/**
 * @brief Free memory for database
 *
 * @param db Database storage
 */
void DatabaseClose(Database *db);

#endif /* __DATABASE_H__ */
