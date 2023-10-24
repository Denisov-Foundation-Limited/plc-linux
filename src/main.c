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
#include <db/database.h>

int main(const int argc, const char **argv)
{
    char    log_path[STR_LEN] = "./data/log/";
    char    cfg_path[STR_LEN] = "./data/configs/";
    char    db_path[STR_LEN] = "./data/db/";
    bool    ftest_start = false;

    if (argc > 1) {
        for (unsigned i = 1; i < argc; i++) {
            if (!strcmp(argv[i], "--ftest")) {
                ftest_start = true;
            } else if (!strcmp(argv[i], "--configs")) {
                strncpy(cfg_path, argv[i + 1], STR_LEN);
            } else if (!strcmp(argv[i], "--db")) {
                strncpy(db_path, argv[i + 1], STR_LEN);
            } else if (!strcmp(argv[i], "--log")) {
                strncpy(log_path, argv[i + 1], STR_LEN);
            } else if (!strcmp(argv[i], "?")) {
                printf("Help information:\n");
                printf("\t--configs [:path]\tPath to Configs directory\n");
                printf("\t--db [:path]\t\tPath to Database directory\n");
                printf("\t--log [:path]\t\tPath to Log directory\n");
                printf("\t--ftest\t\t\tStart factory test\n");
                return 0;
            }
        }
    }

    LogPathSet(log_path);
    DatabasePathSet(db_path);

    Log(LOG_TYPE_INFO, "MAIN", "Starting application");

    if (!GpioInit()) {
        Log(LOG_TYPE_ERROR, "MAIN", "Failed to init GPIO");
        return -1;
    }

    if (!ConfigsRead(cfg_path)) {
        Log(LOG_TYPE_ERROR, "MAIN", "Failed to load configs");
        return -1;
    }

    Log(LOG_TYPE_INFO, "MAIN", "Configs was readed");

    if (ftest_start) {
        if (!FactoryTestStart()) {
            Log(LOG_TYPE_ERROR, "MAIN", "Failed to start Factory Test");
        }
        Log(LOG_TYPE_INFO, "MAIN", "Exiting!");
        return 0;
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
