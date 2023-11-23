/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __EXTENDERS_H__
#define __EXTENDERS_H__

#include <stdbool.h>

#include <utils/utils.h>

typedef enum {
    EXT_TYPE_PCF_8574,
    EXT_TYPE_MCP_23017,
    EXT_TYPE_ADS_1115
} ExtenderType;

typedef struct {
    char            name[SHORT_STR_LEN];
    ExtenderType    type;
    unsigned        addr;
    unsigned        base;
    bool            enabled;
} Extender;

/**
 * @brief Make new Extender object
 * 
 * @param name Name of extender
 * @param type Type of extender
 * @param addr I2C addr of extender
 * @param base GPIO base number
 * @param enabled Enabled status of extender
 * 
 * @return Extender object
 */
Extender *ExtenderNew(const char *name, ExtenderType type, unsigned addr, unsigned base, bool enabled);

/**
 * @brief Add new I2C extender
 * 
 * @param ext New extender
 * @param err Addition error if exists
 * 
 * @return Result of extender addition
 */
bool ExtenderAdd(const Extender *ext, char *err);

#endif /* __EXTENDERS_H__ */
