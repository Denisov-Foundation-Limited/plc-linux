/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <stdlib.h>

#include <utils/utils.h>

bool UtilsURIParse(const char *url, GList **params)
{
    bool ret = false;

    char **uri = g_strsplit(url, "?", 0);
    if (uri != NULL && uri[1] != NULL) {
        char **parts = g_strsplit(uri[1], "&", 0);
        if (parts != NULL) {
            for (unsigned i = 0; i < g_strv_length(parts); i++) {
                char **p = g_strsplit(parts[i], "=", 0);

                if (p[0] != NULL && p[1] != NULL) {
                    UtilsReqParam *param = (UtilsReqParam *)malloc(sizeof(UtilsReqParam));
                    strncpy(param->name, p[0], SHORT_STR_LEN);
                    strncpy(param->value, p[1], SHORT_STR_LEN);
                    *params = g_list_append(*params, (void *)param);
                    ret = true;
                }

                if (p != NULL) {
                    g_strfreev(p);
                }
            }

            if (parts != NULL) {
                g_strfreev(parts);
            }
        }
        if (uri != NULL) {
            g_strfreev(uri);
        }
    }

    return ret;
}