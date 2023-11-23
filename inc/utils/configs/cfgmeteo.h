/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __CFG_METEO_H__
#define __CFG_METEO_H__

#include <stdbool.h>

#include <jansson.h>

/**
 * @brief Loading meteo configurations
 * 
 * @param data JSON configs data
 * 
 * @return True/False as result of loading configs 
 */
bool CfgMeteoLoad(json_t *data);

#endif /* __CFG_METEO_H__ */
