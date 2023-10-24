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

typedef enum {
    LOG_TYPE_INFO,
    LOG_TYPE_WARN,
    LOG_TYPE_ERROR
} LogType;

typedef struct {
    char    full_msg[EXT_STR_LEN];
    char    date_str[SHORT_STR_LEN];
} LogData;

/**
 * @brief Set log file folder
 * 
 * @param path Log file destination folder
 */
void LogPathSet(const char *path);

/**
 * @brief Logging message
 * 
 * @param type Log type
 * @param module Code module
 * @param msg Logging message
 * 
 * @return true/false as result of saving message to disk
 */
bool Log(const LogType type, const char *module, const char *msg);

/**
 * @brief Logging formatted message
 * 
 * @param type Log type
 * @param module Code module
 * @param args Formatted log message
 * 
 * @return true/false as result of saving message to disk
 */
#define LogF(type, module, args...) \
    do { \
        char buf[EXT_STR_LEN]; \
        snprintf(buf, EXT_STR_LEN, ##args); \
        Log(type, module, buf); \
    } while(0)

#endif /* __LOG_H__ */