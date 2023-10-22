/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <glib.h>

#include <core/extenders.h>

#ifdef __ARM__
#include <wiringPi.h>
#endif

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static GList *exts = NULL;

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool ExtenderAdd(const Extender *ext, char *err)
{
#ifdef __ARM__
    switch (ext->type) {
        case EXT_TYPE_PCF_8574:
            if (pcf8574Setup(ext->base, ext->addr) < 0) {
                return false;
            }
            break;

        case EXT_TYPE_MCP_23017:
            if (mcp23017Setup(ext->base, ext->addr) < 0) {
                return false;
            }
            break;

        case EXT_TYPE_ADS_1115:
            if (ads1115Setup(ext->base, ext->addr) < 0) {
                return false;
            }
            break;
    }
#endif
    exts = g_list_append(exts, (void *)ext);
    return true;
}
