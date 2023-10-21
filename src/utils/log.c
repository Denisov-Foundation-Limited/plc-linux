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

bool Log(const LogType type, const char *module, const char *msg)
{
    char        full_msg[EXT_STR_LEN];
    char        log_type[STR_LEN];
    char        time_str[STR_LEN];
    char        date_str[STR_LEN];
    long int    s_time;
    struct      tm *cur_time;

   s_time = time(NULL);
   cur_time = localtime(&s_time);

   snprintf(time_str, STR_LEN, "%d:%d:%d", cur_time->tm_hour, cur_time->tm_min, cur_time->tm_sec);
   snprintf(date_str, STR_LEN, "%d:%d:%d", cur_time->tm_year, cur_time->tm_mon, cur_time->tm_mday);

    switch (type) {
        case LOG_TYPE_INFO:
            strncpy(log_type, "INFO", STR_LEN);
            break;

        case LOG_TYPE_WARN:
            strncpy(log_type, "WARN", STR_LEN);
            break;

        case LOG_TYPE_ERROR:
            strncpy(log_type, "ERROR", STR_LEN);
            break;
    }

#pragma GCC diagnostic ignored "-Wformat-truncation"
    snprintf(full_msg, STR_LEN, "[%s][%s][%s][%s] %s\n", date_str, time_str, log_type, module, msg);
#pragma GCC diagnostic pop

    printf("%s", full_msg);

    return true;
}