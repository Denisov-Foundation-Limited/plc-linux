/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <threads.h>

#include <controllers/security.h>
#include <utils/log.h>
#include <core/onewire.h>

static GList *security = NULL;

static int AlarmThread(void *data)
{
    for (;;) {
        for (GList *c = security; c != NULL; c = c->next) {
            SecurityController *ctrl = (SecurityController *)c->data;
            if (ctrl->alarm) {
                if (ctrl->_buzzer) {
                    ctrl->_buzzer = false;
                    if (!GpioPinWrite(ctrl->gpio[SECURITY_GPIO_BUZZER], false)) {
                        Log(LOG_TYPE_ERROR, "SECURITY", "Failed to write to Buzzer gpio");
                    }
                    if (!GpioPinWrite(ctrl->gpio[SECURITY_GPIO_ALARM_LED], false)) {
                        Log(LOG_TYPE_ERROR, "SECURITY", "Failed to write to Alarm LED gpio");
                    }
                } else {
                    ctrl->_buzzer = true;
                    if (!GpioPinWrite(ctrl->gpio[SECURITY_GPIO_BUZZER], true)) {
                        Log(LOG_TYPE_ERROR, "SECURITY", "Failed to write to Buzzer gpio");
                    }
                    if (!GpioPinWrite(ctrl->gpio[SECURITY_GPIO_ALARM_LED], true)) {
                        Log(LOG_TYPE_ERROR, "SECURITY", "Failed to write to Alarm LED gpio");
                    }
                }
            }
        }
        thrd_sleep(&(struct timespec){ .tv_nsec = 500000 }, NULL);
    }

    return 0;
}

static int SensorsThread(void *data)
{
    for (;;) {
        for (GList *c = security; c != NULL; c = c->next) {
            SecurityController *ctrl = (SecurityController *)c->data;
            if (ctrl->status) {
                for (GList *s = ctrl->sensors; s != NULL; s = s->next) {
                    SecuritySensor *sensor = (SecuritySensor *)s->data;
                    if (!sensor->detected) {
                        switch (sensor->type) {
                            case SECURITY_SENSOR_MICRO_WAVE:
                                if (!GpioPinRead(sensor->gpio)) {
                                    sensor->detected = true;
                                }
                                break;

                            case SECURITY_SENSOR_PIR:
                                if (GpioPinRead(sensor->gpio)) {
                                    sensor->detected = true;
                                }
                                break;

                            case SECURITY_SENSOR_REED:
                                if (GpioPinRead(sensor->gpio)) {
                                    sensor->detected = true;
                                }
                                break;
                        }

                        if (sensor->detected) {
                            LogF(LOG_TYPE_INFO, "SECURITY", "Security sensor \"%s\" detected!", sensor->name);

                            if (sensor->sms) {
                                Log(LOG_TYPE_INFO, "SECURITY", "Alarm message was sended to phone");
                            }

                            if (sensor->telegram) {
                                Log(LOG_TYPE_INFO, "SECURITY", "Alarm message was sended to telegram");
                            }

                            if (sensor->alarm && !ctrl->alarm) {
                                ctrl->alarm = true;
                                LogF(LOG_TYPE_INFO, "SECURITY", "Alarm was started");
                                if (!GpioPinWrite(ctrl->gpio[SECURITY_GPIO_ALARM_RELAY], true)) {
                                    Log(LOG_TYPE_ERROR, "SECURITY", "Failed to write to Alarm relay gpio");
                                }
                            }
                        }
                    }
                }
            }
        }

        thrd_sleep(&(struct timespec){ .tv_sec = 1 }, NULL);
    }

    return 0;
}

static int KeysThread(void *data)
{
    GList   **cur_keys = NULL;
    bool    ow_error = false;

    for (;;) {
        if (!OneWireKeysRead(cur_keys)) {
            if (!ow_error) {
                ow_error = true;
                Log(LOG_TYPE_ERROR, "SECURITY", "Failed to read iButton codes");
            }
            if (cur_keys != NULL) {
                g_list_free(*cur_keys);
            }
            thrd_sleep(&(struct timespec){ .tv_sec = 2 }, NULL);
            continue;
        } else {
            if (ow_error) {
                ow_error = false;
                Log(LOG_TYPE_INFO, "SECURITY", "Successfully readed iButton codes");
            }
        }

        for (GList *k = *cur_keys; k != NULL; k = k->next) {
            OneWireData *data = (OneWireData *)k->data;
            bool found = false;

            for (GList *c = security; c != NULL; c = c->next) {
                SecurityController *ctrl = (SecurityController *)c->data;

                for (GList *ck = ctrl->keys; ck != NULL; ck = ck->next) {
                    SecurityKey *skey = (SecurityKey *)ck->data;
                    if (!strcmp(data->value, skey->value)) {
                        found = true;
                        if (!SecurityStatusSet(ctrl, !SecurityStatusGet(ctrl))) {
                            LogF(LOG_TYPE_ERROR, "SECURITY", "Failed to switch security status by iButton for \"%s\" controller", ctrl->name);
                        }
                        break;
                    }
                }
            }

            if (!found) {
                LogF(LOG_TYPE_ERROR, "SECURITY", "Unknown security key: \"%s\"", data->value);
            }

            if (data != NULL) {
                free(data);
            }
        }

        if (*cur_keys != NULL) {
            g_list_free(*cur_keys);
        }

        thrd_sleep(&(struct timespec){ .tv_sec = 3 }, NULL);
    }

    return 0;
}

void SecurityControllerAdd(const SecurityController *ctrl)
{
    security = g_list_append(security, (void *)ctrl);
}

SecurityController *SecurityControllerGet(const char *name)
{
    for (GList *c = security; c != NULL; c = c->next) {
        SecurityController *ctrl = (SecurityController *)c->data;
        if (!strcmp(ctrl->name, name)) {
            return ctrl;
        }
    }
    return NULL;
}

GList **SecurityControllersGet()
{
    return &security;
}

bool SecurityControllersStart()
{
    thrd_t  sens_th, keys_th, alrm_th;
    int     sens_res, keys_res, alrm_res;

    if (g_list_length(security) == 0) {
        return true;
    }

    Log(LOG_TYPE_INFO, "SECURITY", "Starting Security controllers");

    thrd_create(&sens_th, &SensorsThread, NULL);
    thrd_create(&keys_th, &KeysThread, NULL);
    thrd_create(&alrm_th, &AlarmThread, NULL);

    thrd_join(sens_th, &sens_res);
    thrd_join(keys_th, &keys_res);
    thrd_join(alrm_th, &alrm_res);

    if (sens_res < 0 || keys_res < 0 || alrm_res < 0) {
        return false;
    }

    return true;
}

bool SecurityStatusSet(SecurityController *ctrl, bool status)
{
    ctrl->status = status;

    if (!status) {
        ctrl->alarm = false;

        if (!GpioPinWrite(ctrl->gpio[SECURITY_GPIO_BUZZER], false)) {
            Log(LOG_TYPE_ERROR, "SECURITY", "Failed to write to Buzzer gpio");
            return false;
        }
        if (!GpioPinWrite(ctrl->gpio[SECURITY_GPIO_ALARM_LED], false)) {
            Log(LOG_TYPE_ERROR, "SECURITY", "Failed to write to Alarm LED gpio");
            return false;
        }
        if (!GpioPinWrite(ctrl->gpio[SECURITY_GPIO_ALARM_RELAY], false)) {
            Log(LOG_TYPE_ERROR, "SECURITY", "Failed to write to Alarm relay gpio");
            return false;
        }
        if (!GpioPinWrite(ctrl->gpio[SECURITY_GPIO_STATUS_LED], false)) {
            Log(LOG_TYPE_ERROR, "SECURITY", "Failed to write to Status LED gpio");
            return false;
        }
        LogF(LOG_TYPE_INFO, "SECURITY", "Security controller \"%s\" disabled", ctrl->name);
    } else {
        if (!GpioPinWrite(ctrl->gpio[SECURITY_GPIO_STATUS_LED], true)) {
            Log(LOG_TYPE_ERROR, "SECURITY", "Failed to write to Status LED gpio");
            return false;
        }
        LogF(LOG_TYPE_INFO, "SECURITY", "Security controller \"%s\" enabled", ctrl->name);
    }

    return true;
}

bool SecurityStatusGet(const SecurityController *ctrl)
{
    return ctrl->status;
}

void SecuritySensorAdd(SecurityController *ctrl, const SecuritySensor *sensor)
{
    ctrl->sensors = g_list_append(ctrl->sensors, (void *)sensor);
}

SecuritySensor *SecuritySensorGet(const SecurityController *ctrl, const char *name)
{
    for (GList *c = ctrl->sensors; c != NULL; c = c->next) {
        SecuritySensor *sensor = (SecuritySensor *)c->data;
        if (!strcmp(sensor->name, name)) {
            return sensor;
        }
    }
    return NULL;
}

GList **SecuritySensorsGet(SecurityController *ctrl)
{
    return &ctrl->sensors;
}

void SecurityKeyAdd(SecurityController *ctrl, const SecurityKey *key)
{
    ctrl->keys = g_list_append(ctrl->keys, (void *)key);
}
