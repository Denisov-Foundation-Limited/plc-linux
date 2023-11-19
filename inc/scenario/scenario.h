/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __SCENARIO_H__
#define __SCENARIO_H__

#include <stdbool.h>

#include <utils/utils.h>

typedef enum {
    SCENARIO_IN_HOME,
    SCENARIO_OUT_HOME
} ScenarioType;

typedef enum {
    SECURITY_CTRL_SOCKET
} ScenarioCtrlType;

typedef struct {
    char    name[SHORT_STR_LEN];
    bool    status;
} ScenarioSocket;

typedef struct {
    ScenarioType        type;
    unsigned            unit;
    ScenarioCtrlType    ctrl;
    ScenarioSocket      socket;
} Scenario;

/**
 * @brief Add new scenario to list
 * 
 * @param scenario Scenario with params
 */
void ScenarioAdd(Scenario *scenario);

/**
 * @brief Start all scenario by type
 * 
 * @param type Scenario type
 */
bool ScenarioStart(ScenarioType type);

#endif /* __SCENARIO_H__ */
