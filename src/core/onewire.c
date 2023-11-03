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
#include <stdio.h>

#include <core/onewire.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool OneWireDevicesList(GList **devices)
{
    GError      *error = NULL;
    const char  *file_name;
    GDir        *dir = g_dir_open(ONE_WIRE_PATH, 0, &error);

    if (dir == NULL) {
        g_error_free(error);
        return false;
    }

    while ((file_name = g_dir_read_name(dir))) {
        char **parts = g_strsplit(file_name, "-", 0);

        if (parts[0] != NULL && parts[1] != NULL) {
            OneWireData *data = (OneWireData *)malloc(sizeof(OneWireData));
            strncpy(data->value, file_name, SHORT_STR_LEN);
            *devices = g_list_append(*devices, (void *)data);
        }

        if (parts != NULL) {
            g_strfreev(parts);
        }
    }

    g_dir_close(dir);

    return true;
}

bool OneWireKeysRead(GList **keys)
{
    GError      *error = NULL;
    const char  *file_name;
    GDir        *dir = g_dir_open(ONE_WIRE_PATH, 0, &error);

    if (dir == NULL) {
        g_error_free(error);
        return false;
    }

    while ((file_name = g_dir_read_name(dir))) {
        char **parts = g_strsplit(file_name, "-", 0);

        if (parts[0] != NULL && parts[1] != NULL) {
            if (!strcmp(parts[0], ONE_WIRE_IBUTTON_PREFIX)) {
                OneWireData *data = (OneWireData *)malloc(sizeof(OneWireData));
                strncpy(data->value, parts[1], SHORT_STR_LEN);
                *keys = g_list_append(*keys, data);
            }
        }

        if (parts != NULL) {
            g_strfreev(parts);
        }
    }

    g_dir_close(dir);

    return true;
}

bool OneWireTempRead(const char *id, float *temp)
{
    char    buf[STR_LEN];
    char    file_name[STR_LEN];
    FILE    *file;

    snprintf(file_name, STR_LEN, "%s/%s-%s/temperature", ONE_WIRE_PATH, ONE_WIRE_DS18B20_PREFIX, id);

    file = fopen(file_name, "r");
    if (file == NULL) {
        return false;
    }
    fscanf(file, "%s", buf);
    fclose(file);

    *temp = strtof(buf, NULL);
    *temp /= 1000;

    if (*temp <= ONE_WIRE_INVALID_TEMP) {
        return false;
    }

    return true;
}
