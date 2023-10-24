/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <string.h>

#include <utils/configs.h>
#include <utils/utils.h>
#include <utils/log.h>
#include <controllers/controllers.h>
#include <ftest/ftest.h>
#include <core/gpio.h>
#include <net/server.h>

int main(const int argc, const char **argv)
{
    LogPathSet("./data/log/");
    Log(LOG_TYPE_INFO, "MAIN", "Starting application");

    if (!GpioInit()) {
        Log(LOG_TYPE_ERROR, "MAIN", "Failed to init GPIO");
        return -1;
    }

    if (!ConfigsRead("./data/configs/")) {
        Log(LOG_TYPE_ERROR, "MAIN", "Failed to load configs");
        return -1;
    }

    Log(LOG_TYPE_INFO, "MAIN", "Configs was readed");

    if (argc > 1) {
        if (!strcmp(argv[1], "--ftest")) {
            if (!FactoryTestStart()) {
                Log(LOG_TYPE_ERROR, "MAIN", "Failed to start Factory Test");
            }
            Log(LOG_TYPE_INFO, "MAIN", "Exiting!");
            return 0;
        }
    }

    Log(LOG_TYPE_INFO, "MAIN", "Starting controllers");

    if (!ControllersStart()) {
        Log(LOG_TYPE_ERROR, "MAIN", "Failed to start controllers");
        return -1;
    }

    Log(LOG_TYPE_INFO, "MAIN", "Starting Web Server");

    if (!WebServerStart()) {
        Log(LOG_TYPE_ERROR, "MAIN", "Failed to start web server");
        return -1;
    }

    Log(LOG_TYPE_INFO, "MAIN", "Exiting!");

    return 0;
}