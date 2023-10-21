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

#include <glib.h>
#include <jansson.h>

#include <utils/configs.h>
#include <utils/utils.h>
#include <utils/log.h>
#include <controllers/controllers.h>

int main()
{
    Log(LOG_TYPE_INFO, "MAIN", "Starting application");

    if (!ConfigsRead("./data/configs/")) {
        Log(LOG_TYPE_ERROR, "MAIN", "Failed to load configs");
        return -1;
    }

    Log(LOG_TYPE_INFO, "MAIN", "Configs was readed");
    Log(LOG_TYPE_INFO, "MAIN", "Starting controllers");

    if (!ControllersStart()) {
        Log(LOG_TYPE_ERROR, "MAIN", "Failed to start controllers");
        return -1;
    }

    Log(LOG_TYPE_INFO, "MAIN", "Exiting!");

    return 0;
}