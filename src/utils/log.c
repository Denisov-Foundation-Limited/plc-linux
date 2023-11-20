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
    struct tm   *cur_time = UtilsLinuxTimeGet();
    char        date_str[STR_LEN];
    GString     *text = g_string_new("");

    g_string_append_printf(text, "[%d.%d.%d][%d:%d:%d]",
        cur_time->tm_year, cur_time->tm_mon, cur_time->tm_mday,
        cur_time->tm_hour, cur_time->tm_min, cur_time->tm_sec
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
    struct tm   *cur_time = UtilsLinuxTimeGet();
    char        date_str[STR_LEN];
    GString     *text = NULL;

    snprintf(date_str, STR_LEN, "%d.%d.%d.log", cur_time->tm_year, cur_time->tm_mon, cur_time->tm_mday);

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
