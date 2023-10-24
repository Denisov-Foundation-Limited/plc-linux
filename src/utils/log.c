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

static int LogThread(void *data)
{
    LogData *log_data = (LogData *)data;

    mtx_lock(&log_mtx);

    if (!LogSaveToFile(log_data->date_str, log_data->full_msg)) {
        mtx_unlock(&log_mtx);
        printf("Failed to save log message to file\n");
        free(log_data);
        return -1;
    }
    free(log_data);

    mtx_unlock(&log_mtx);
    return 0;
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
    
    char        log_type[SHORT_STR_LEN];
    char        time_str[SHORT_STR_LEN];
    char        date_str[SHORT_STR_LEN];
    char        full_msg[EXT_STR_LEN];
    long int    s_time;
    struct      tm *cur_time;
    thrd_t      th;
    LogData     *data = (LogData *)malloc(sizeof(LogData));

   s_time = time(NULL);
   cur_time = localtime(&s_time);
   cur_time->tm_year += 1900;
   cur_time->tm_mon += 1;

   snprintf(time_str, SHORT_STR_LEN, "%d:%d:%d", cur_time->tm_hour, cur_time->tm_min, cur_time->tm_sec);
   snprintf(date_str, SHORT_STR_LEN, "%d.%d.%d", cur_time->tm_year, cur_time->tm_mon, cur_time->tm_mday);

    switch (type) {
        case LOG_TYPE_INFO:
            strncpy(log_type, "INFO", SHORT_STR_LEN);
            break;

        case LOG_TYPE_WARN:
            strncpy(log_type, "WARN", SHORT_STR_LEN);
            break;

        case LOG_TYPE_ERROR:
            strncpy(log_type, "ERROR", SHORT_STR_LEN);
            break;
    }

#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(data->full_msg, EXT_STR_LEN, "[%s][%s][%s][%s] %s\n", date_str, time_str, log_type, module, msg);
#pragma GCC diagnostic pop

    printf("%s", data->full_msg);

    snprintf(data->date_str, SHORT_STR_LEN, "%d-%d-%d", cur_time->tm_year, cur_time->tm_mon, cur_time->tm_mday);
    thrd_create(&th, LogThread, (void *)data);

    return true;
}