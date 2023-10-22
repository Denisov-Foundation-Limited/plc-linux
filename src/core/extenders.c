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
    switch (ext->type) {
        case EXT_TYPE_PCF_8574:
            break;

        case EXT_TYPE_MCP_23017:
            break;

        case EXT_TYPE_ADS_1115:
            break;
    }

    exts = g_list_append(exts, (void *)ext);
    return true;
}
