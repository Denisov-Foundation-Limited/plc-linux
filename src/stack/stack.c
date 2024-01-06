/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <stack/stack.h>
#include <utils/log.h>
#include <stack/rpc.h>

#include <threads.h>
#include <stdlib.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct {
    GList   *units;
} Stack = {
    .units = NULL,
};

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static void UnitsStatusCheck()
{
    for (GList *u = *StackUnitsGet(); u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;

        if (RpcUnitStatusCheck(unit->id)) {
            if (!unit->active) {
                unit->active = true;
                LogF(LOG_TYPE_INFO, "STACK", "Unit \"%s\" is online", unit->name);
            }
        } else {
            if (unit->active) {
                unit->active = false;
                LogF(LOG_TYPE_INFO, "STACK", "Unit \"%s\" is offline", unit->name);
            }
        }
    }
}

static void SecurityControllersUpdate()
{
    GList   *units = NULL;
    bool    master_status = false;
    bool    master_alarm = false;
    bool    slave_status = false;
    bool    slave_alarm = false;

    if (!RpcSecurityStatusGet(RPC_DEFAULT_UNIT, &master_status)) {
        LogF(LOG_TYPE_ERROR, "STACK", "Failed to get Security status from Unit %d", 0);
        return;
    }

    if (!RpcSecurityAlarmGet(RPC_DEFAULT_UNIT, &master_alarm)) {
        LogF(LOG_TYPE_ERROR, "STACK", "Failed to get Security alarm from Unit %d", 0);
        return;
    }

    StackActiveUnitsGet(&units);

    for (GList *u = units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;

        if (unit->id == RPC_DEFAULT_UNIT) {
            continue;
        }

        if (!RpcSecurityStatusGet(unit->id, &slave_status)) {
            if (!unit->error) {
                unit->error = true;
                LogF(LOG_TYPE_ERROR, "STACK", "Failed to get Security status from Unit %d", unit->id);
                continue;
            }
        } else {
            if (unit->error) {
                unit->error = false;
                LogF(LOG_TYPE_INFO, "STACK", "Successfully get Security status from Unit %d", unit->id);
            }
        }

        if (slave_status != master_status) {
            if (!RpcSecurityStatusSet(unit->id, master_status)) {
                if (!unit->error) {
                    unit->error = true;
                    LogF(LOG_TYPE_ERROR, "STACK", "Failed to set Security status from Unit %d", unit->id);
                    continue;
                }
            } else {
                if (unit->error) {
                    unit->error = false;
                    LogF(LOG_TYPE_INFO, "STACK", "Successfully set Security status from Unit %d", unit->id);
                }
            }
        }

        if (!RpcSecurityAlarmGet(unit->id, &slave_alarm)) {
            if (!unit->error) {
                unit->error = true;
                LogF(LOG_TYPE_ERROR, "STACK", "Failed to get Security alarm from Unit %d", unit->id);
                continue;
            }
        } else {
            if (unit->error) {
                unit->error = false;
                LogF(LOG_TYPE_INFO, "STACK", "Successfully get Security alarm from Unit %d", unit->id);
            }
        }

        if (slave_alarm && !master_alarm) {
            if (!RpcSecurityAlarmSet(RPC_DEFAULT_UNIT, true)) {
                if (!unit->error) {
                    unit->error = true;
                    LogF(LOG_TYPE_ERROR, "STACK", "Failed to set Security alarm to Unit %d", RPC_DEFAULT_UNIT);
                    continue;
                }
            } else {
                master_alarm = true;
                if (unit->error) {
                    unit->error = false;
                    LogF(LOG_TYPE_INFO, "STACK", "Successfully set Security alarm to Unit %d", RPC_DEFAULT_UNIT);
                }
            }
        }

        if (!slave_alarm && master_alarm) {
            if (!RpcSecurityAlarmSet(unit->id, true)) {
                if (!unit->error) {
                    unit->error = true;
                    LogF(LOG_TYPE_ERROR, "STACK", "Failed to set Security alarm to Unit %d", unit->id);
                    continue;
                }
            } else {
                if (unit->error) {
                    unit->error = false;
                    LogF(LOG_TYPE_INFO, "STACK", "Successfully set Security alarm to Unit %d", unit->id);
                }
            }
        }
    }

    g_list_free(units);
    units = NULL;
}

static int StackThread(void *data)
{
    for (;;) {
        UnitsStatusCheck();
        SecurityControllersUpdate();
        UtilsSecSleep(3);
    }
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

unsigned StackUnitIdGet(const char *name)
{
    for (GList *u = Stack.units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;
        if (!strcmp(unit->name, name)) {
            return unit->id;
        }
    }
    return 0;
}

StackUnit *StackUnitNew(unsigned id, const char *name, const char *ip, unsigned port)
{
    StackUnit *unit = (StackUnit *)malloc(sizeof(StackUnit));

    unit->id = id;
    unit->error = false;
    strncpy(unit->name, name, SHORT_STR_LEN);
    strncpy(unit->ip, ip, SHORT_STR_LEN);
    unit->active = false;
    unit->port = port;

    return unit;
}

StackUnit *StackUnitNameGet(const char *name)
{
    for (GList *u = Stack.units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;
        if (!strcmp(unit->name, name)) {
            return unit;
        }
    }
    return NULL;
}

StackUnit *StackUnitGet(unsigned id)
{
    for (GList *u = Stack.units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;
        if (unit->id == id) {
            return unit;
        }
    }
    return NULL;
}

void StackUnitAdd(StackUnit *unit)
{
    Stack.units = g_list_append(Stack.units, (void *)unit);
}

bool StackUnitNameCheck(const char *name)
{
    for (GList *u = Stack.units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;
        if (!strcmp(unit->name, name)) {
            return true;
        }
    }
    return false;
}

void StackActiveUnitsGet(GList **units)
{
    for (GList *u = Stack.units; u != NULL; u = u->next) {
        StackUnit *unit = (StackUnit *)u->data;
        if (unit->active) {
            *units = g_list_append(*units, (void *)unit);
        }
    }
}

GList **StackUnitsGet()
{
    return &Stack.units;
}

bool StackStart()
{
    thrd_t  stk_th;

    Log(LOG_TYPE_INFO, "STACK", "Starting Stack monitoring");

    if (thrd_create(&stk_th, &StackThread, NULL) != thrd_success) {
        return false;
    }
    if (thrd_detach(stk_th) != thrd_success) {
        return false;
    }

    return true;
}
