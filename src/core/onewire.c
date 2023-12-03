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
/*                            PRIVATE TYPES                          */
/*                                                                   */
/*********************************************************************/

typedef enum {
    ONE_WIRE_IBUTTONS,
    ONE_WIRE_TEMPS,
    ONE_WIRE_ALL
} OneWireDeviceType;

/*********************************************************************/
/*                                                                   */
/*                          PRIVATE FUNCTIONS                        */
/*                                                                   */
/*********************************************************************/

static bool OneWireDevicesGet(OneWireDeviceType type, GList **devices)
{
    char    buf[STR_LEN];
    FILE    *fd = NULL;

    fd = fopen(ONE_WIRE_SLAVES_PATH, "r");
    if (fd == NULL) {
        return false;
    }

    while (fgets(buf, STR_LEN, fd)) {
        buf[strlen(buf)-1] = '\0';

        char **parts = g_strsplit(buf, "-", 0);

        if (parts[0] != NULL && parts[1] != NULL) {
            OneWireData *data = NULL;

            switch (type) {
                case ONE_WIRE_IBUTTONS:
                    if (!strcmp(parts[0], ONE_WIRE_IBUTTON_PREFIX)) {
                        data = (OneWireData *)malloc(sizeof(OneWireData));
                        strncpy(data->value, parts[1], SHORT_STR_LEN);
                        *devices = g_list_append(*devices, data);
                    }
                    break;

                case ONE_WIRE_TEMPS:
                    if (!strcmp(parts[0], ONE_WIRE_DS18B20_PREFIX)) {
                        data = (OneWireData *)malloc(sizeof(OneWireData));
                        strncpy(data->value, parts[1], SHORT_STR_LEN);
                        *devices = g_list_append(*devices, data);
                    }
                    break;

                case ONE_WIRE_ALL:
                    data = (OneWireData *)malloc(sizeof(OneWireData));
                    strncpy(data->value, parts[1], SHORT_STR_LEN);
                    *devices = g_list_append(*devices, data);
                    break;
            }
        }

        g_strfreev(parts);
    }

    fclose(fd);

    return true;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool OneWireDevicesList(GList **devices)
{
    return OneWireDevicesGet(ONE_WIRE_ALL, devices);
}

bool OneWireKeysRead(GList **keys)
{
    return OneWireDevicesGet(ONE_WIRE_IBUTTONS, keys);
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
