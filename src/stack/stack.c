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

static int StackThread(void *data)
{
    for (;;) {
        UtilsSecSleep(5);
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
    strncpy(unit->name, name, SHORT_STR_LEN);
    strncpy(unit->ip, ip, SHORT_STR_LEN);
    if (id == 0) {
        unit->active = true;
    } else {
        unit->active = false;
    }

    return unit;
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
    thrd_create(&stk_th, &StackThread, NULL);

    return true;
}
