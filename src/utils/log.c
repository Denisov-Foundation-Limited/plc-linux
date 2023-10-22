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

#include <utils/log.h>
#include <utils/utils.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static char log_path[STR_LEN] = {0};

/*********************************************************************/
/*                                             log_path                      */
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
    char        full_msg[EXT_STR_LEN];
    char        log_type[SHORT_STR_LEN];
    char        time_str[SHORT_STR_LEN];
    char        date_str[SHORT_STR_LEN];
    long int    s_time;
    struct      tm *cur_time;

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
    snprintf(full_msg, EXT_STR_LEN, "[%s][%s][%s][%s] %s\n", date_str, time_str, log_type, module, msg);
#pragma GCC diagnostic pop

    printf("%s", full_msg);

    snprintf(date_str, SHORT_STR_LEN, "%d-%d-%d", cur_time->tm_year, cur_time->tm_mon, cur_time->tm_mday);

    if (!LogSaveToFile(date_str, full_msg)) {
        printf("Failed to save log message to file\n");
    }

    return true;
}