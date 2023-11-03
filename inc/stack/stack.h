/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __STACK_H__
#define __STACK_H__

#include <utils/utils.h>

#include <stdbool.h>

typedef struct {
    unsigned    id;
    char        name[SHORT_STR_LEN];
    char        ip[SHORT_STR_LEN];
    unsigned    port;
    bool        active;
} StackUnit;

StackUnit *StackUnitNew(unsigned id, const char *name, const char *ip, unsigned port);

void StackUnitAdd(StackUnit *unit);

unsigned StackUnitIdGet(const char *name);

StackUnit *StackUnitGet(unsigned id);

bool StackUnitNameCheck(const char *name);

void StackActiveUnitsGet(GList **units);

GList **StackUnitsGet();

bool StackStart();

#endif /* __STACK_H__ */
