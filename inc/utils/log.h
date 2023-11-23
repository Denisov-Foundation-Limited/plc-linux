/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __LOG_H__
#define __LOG_H__

#include <stdbool.h>
#include <stdio.h>

#include <utils/utils.h>

#include <glib-2.0/glib.h>

typedef enum {
    LOG_TYPE_INFO,
    LOG_TYPE_WARN,
    LOG_TYPE_ERROR
} LogType;

/**
 * @brief Set log file folder
 * 
 * @param path Log file destination folder
 */
void LogPathSet(const char *path);

/**
 * @brief Logging message to console and file
 * 
 * @param type Log type
 * @param module Code module
 * @param msg Logging message
 * 
 * @return true/false as result of saving message to disk
 */
bool Log(const LogType type, const char *module, const char *msg);

/**
 * @brief Logging formatted message to console and file
 * 
 * @param type Log type
 * @param module Code module
 * @param args Formatted log message
 * 
 * @return true/false as result of saving message to disk
 */
#define LogF(type, module, ...) \
    do { \
        GString *msg = g_string_new(""); \
        g_string_append_printf(msg, __VA_ARGS__); \
        Log(type, module, msg->str); \
        g_string_free(msg, true); \
    } while(0)

/**
 * @brief Logging message to console
 * 
 * @param type Log type
 * @param module Code module
 * @param msg Logging message
 * 
 * @return true/false as result of saving message to disk
 */
bool LogPrint(const LogType type, const char *module, const char *msg);

/**
 * @brief Logging formatted message
 * 
 * @param type Log type
 * @param module Code module
 * @param args Formatted log message
 * 
 * @return true/false as result of saving message to disk
 */
#define LogPrintF(type, module, ...) \
    do { \
        GString *msg = g_string_new(""); \
        g_string_append_printf(msg, __VA_ARGS__); \
        LogPrint(type, module, msg->str); \
        g_string_free(msg, true); \
    } while(0)

#endif /* __LOG_H__ */
