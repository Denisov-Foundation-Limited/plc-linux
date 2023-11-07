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
    bool        error;
} StackUnit;

/**
 * @brief Make new StackUnit struct
 * 
 * @param id Stack unit ID
 * @param name Stack unit name
 * @param ip Stack unit IP
 * @param port Stack unit port
 * 
 * @return New stack unit struct
 */
StackUnit *StackUnitNew(unsigned id, const char *name, const char *ip, unsigned port);

/**
 * @brief Add new stack unit to storage
 * 
 * @param unit New stack unit
 */
void StackUnitAdd(StackUnit *unit);

/**
 * @brief Get stack unit by name
 * 
 * @param name Stack unit name
 * 
 * @return StackUnit struct pointer
 */
StackUnit *StackUnitNameGet(const char *name);

/**
 * @brief Get stack unit by ID
 * 
 * @param name Stack unit ID
 * 
 * @return StackUnit struct pointer
 */
StackUnit *StackUnitGet(unsigned id);

/**
 * @brief Check stack unit exists by name
 * 
 * @param name Stack unit name
 * 
 * @return True if unit exists
*/
bool StackUnitNameCheck(const char *name);

/**
 * @brief Get all active stack units
 * 
 * @param units Active units list
*/
void StackActiveUnitsGet(GList **units);

/**
 * @brief Get all stack units
 * 
 * @return Active units list
*/
GList **StackUnitsGet();

/**
 * @brief Start stack units monitoring
 * 
 * @return True/False as result of starting monitoring
 */
bool StackStart();

#endif /* __STACK_H__ */
