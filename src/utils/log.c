/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <threads.h>
#include <stdint.h>

#include <utils/log.h>
#include <utils/utils.h>
#include <plc/plc.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static char     log_path[STR_LEN] = {0};
static mtx_t    log_mtx;

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool LogSaveToFile(const char *date, const char *msg)
{
    char    file_name[EXT_STR_LEN];
    FILE    *file;

    snprintf(file_name, EXT_STR_LEN, "%s%s.log", log_path, date);
 
    file = fopen(file_name, "a+");
    if (!file) {
        return false;
    }

    if (fprintf(file, "%s", msg) < 0) {
        fclose(file);
        return false;
    }

    fclose(file);
    return true;
}

GString *LogMakeMsg(const LogType type, const char *module, const char *msg)
{
    PlcTime time;
    GString *text = g_string_new("");

    PlcTimeGet(&time);

    g_string_append_printf(text, "[%4d.%2d.%2d][%2d:%2d:%2d]",
       time.year, time.month, time.day,
       time.hour, time.min, time.sec
    );

    g_string_append_printf(text, "[%s]", module);

    switch (type) {
        case LOG_TYPE_INFO:
            g_string_append_printf(text, "[INFO] %s\n", msg);
            break;

        case LOG_TYPE_WARN:
            g_string_append_printf(text, "[WARN] %s\n", msg);
            break;

        case LOG_TYPE_ERROR:
            g_string_append_printf(text, "[ERROR] %s\n", msg);
            break;
    }

    return text;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void LogPathSet(const char *path)
{
    strncpy(log_path, path, STR_LEN);
}

bool Log(const LogType type, const char *module, const char *msg)
{
    PlcTime time;
    char        date_str[STR_LEN];
    GString     *text = NULL;

    PlcTimeGet(&time);

    snprintf(date_str, STR_LEN, "%4d.%2d.%2d.log", time.year, time.month, time.day);

    text = LogMakeMsg(type, module, msg);
    printf("%s", text->str);

    mtx_lock(&log_mtx);
    if (!LogSaveToFile(date_str, text->str)) {
        mtx_unlock(&log_mtx);
        printf("Failed to save log message to file\n");
        g_string_free(text, true);
        return -1;
    }
    mtx_unlock(&log_mtx);
    g_string_free(text, true);

    return true;
}

bool LogPrint(const LogType type, const char *module, const char *msg)
{
    GString *text = NULL;

    text = LogMakeMsg(type, module, msg);
    printf("%s", text->str);
    g_string_free(text, true);

    return true;
}
