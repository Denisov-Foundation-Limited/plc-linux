/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __CONFIGS_H__
#define __CONFIGS_H__

#include <stdbool.h>

#include <utils/utils.h>

#define CONFIGS_FACTORY_FILE        "factory.json"
#define CONFIGS_CONTROLLERS_FILE    "controllers.json"
#define CONFIGS_PLC_FILE            "plc.json"
#define CONFIGS_SCENARIO_FILE       "scenario.json"

typedef struct {
    char    board[SHORT_STR_LEN];
    char    revision[SHORT_STR_LEN];
} ConfigsFactory;

/**
 * @brief Reading configs from files
 * 
 * @param path Path to config files
 * 
 * @return true/false as result of reading and parsing configs
 */
bool ConfigsRead(const char *path);

#endif /* __CONFIGS_H__ */
