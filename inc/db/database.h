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

#include <sqlite3.h>

typedef struct {
    sqlite3 *base;
} Database;

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
 * @brief Free memory for database
 *
 * @param db Database storage
 */
void DatabaseClose(Database *db);

#endif /* __DATABASE_H__ */
