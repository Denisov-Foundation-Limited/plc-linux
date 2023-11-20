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
#include <net/notifier.h>
#include <db/database.h>
#include <controllers/socket.h>
#include <stack/stack.h>
#include <stack/rpc.h>
#include <scenario/scenario.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct _Security {
    GList           *sensors;
    GList           *keys;
    GpioPin         *gpio[SECURITY_GPIO_MAX];
    mtx_t           sec_mtx;
    bool            status;
    bool            alarm;
    bool            last_alarm;
    bool            sound[SECURITY_SOUND_MAX];
} Security = {
    .sensors = NULL,
    .keys = NULL,
    .status = false,
    .alarm = false,
    .last_alarm = false,
};

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static bool StatusSave(SecurityStatusType type, bool status)
{
    Database    db;
    char        sql[STR_LEN];
    char        con[STR_LEN];

    if (!DatabaseOpen(&db, SECURITY_DB_FILE)) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "SECURITY", "Failed to load Security database");
        return false;
    }

    if (type == SECURITY_SAVE_TYPE_STATUS) {
        snprintf(sql, STR_LEN, "status=%d", (int)status);
    } else {
        snprintf(sql, STR_LEN, "alarm=%d", (int)status);
    }

    if (!DatabaseUpdate(&db, "security", sql, "name=\"controller\"")) {
        DatabaseClose(&db);
        Log(LOG_TYPE_ERROR, "SECURITY", "Failed to update Security database");
        return false;
    }

    DatabaseClose(&db);
    return true;
}

static int StatusSaveThread(void *data)
{
    mtx_lock(&Security.sec_mtx);

    if (!StatusSave(SECURITY_SAVE_TYPE_STATUS, SecurityStatusGet())) {
        mtx_unlock(&Security.sec_mtx);
        return -1;
    }

    mtx_unlock(&Security.sec_mtx);
    return 0;
}

static void AlarmHandler()
{
    if (Security.alarm) {
        if (Security.last_alarm) {
            Security.last_alarm = false;
            if (Security.sound[SECURITY_SOUND_ALARM]) {
                GpioPinWrite(Security.gpio[SECURITY_GPIO_BUZZER], false);
            }
            GpioPinWrite(Security.gpio[SECURITY_GPIO_ALARM_LED], false);
        } else {
            Security.last_alarm = true;
            if (Security.sound[SECURITY_SOUND_ALARM]) {
                GpioPinWrite(Security.gpio[SECURITY_GPIO_BUZZER], true);
            }
            GpioPinWrite(Security.gpio[SECURITY_GPIO_ALARM_LED], true);
        }
    }
}

static int BuzzerThread(void *data)
{
    if (Security.status && Security.sound[SECURITY_SOUND_EXIT]) {
        GpioPinWrite(Security.gpio[SECURITY_GPIO_BUZZER], true);
        UtilsMsecSleep(100);
        GpioPinWrite(Security.gpio[SECURITY_GPIO_BUZZER], false);
        UtilsMsecSleep(100);
        GpioPinWrite(Security.gpio[SECURITY_GPIO_BUZZER], true);
        UtilsMsecSleep(100);
        GpioPinWrite(Security.gpio[SECURITY_GPIO_BUZZER], false);
    } else if (!Security.status && Security.sound[SECURITY_SOUND_ENTER]) {
        GpioPinWrite(Security.gpio[SECURITY_GPIO_BUZZER], true);
        UtilsMsecSleep(300);
        GpioPinWrite(Security.gpio[SECURITY_GPIO_BUZZER], false);
    }
    return 0;
}

static int StatusNotifyThread(void *data)
{
    char    msg[STR_LEN];

    StackUnit *unit = StackUnitGet(RPC_DEFAULT_UNIT);

    if (Security.status) {
        snprintf(msg, STR_LEN, "ОХРАНА:%s+сигнализация+включена", unit->name);
    } else {
        snprintf(msg, STR_LEN, "ОХРАНА:%s+сигнализация+отключена", unit->name);
    }

    if (!NotifierTelegramSend(msg)) {
        Log(LOG_TYPE_ERROR, "SECURITY", "Failed to send status telegram message");
    } else {
        Log(LOG_TYPE_INFO, "SECURITY", "Status message was sended to telegram");
    }

    if (!NotifierSmsSend(msg)) {
        Log(LOG_TYPE_ERROR, "SECURITY", "Failed to send security status sms message");
    } else {
        Log(LOG_TYPE_INFO, "SECURITY", "Status sms was sended to phone");
    }

    return 0;
}

static int AlarmThread(void *data)
{
    for (;;) {
        AlarmHandler();
        UtilsMsecSleep(500);
    }
    return 0;
}

static int SensorsThread(void *data)
{
    char        msg[STR_LEN];
    unsigned    timer = 0;

    for (;;) {
        timer++;

        if (timer > SECURITY_SENSOR_TIME_MAX_SEC) {
            timer = 0;
        }

        if (Security.status) {
            for (GList *s = Security.sensors; s != NULL; s = s->next) {
                SecuritySensor *sensor = (SecuritySensor *)s->data;

                if (sensor->detected) {
                    continue;
                }

                switch (sensor->type) {
                    case SECURITY_SENSOR_MICRO_WAVE:
                        if (!GpioPinRead(sensor->gpio)) {
                            sensor->counter++;
                        }
                        break;

                    case SECURITY_SENSOR_PIR:
                        if (GpioPinRead(sensor->gpio)) {
                            sensor->counter++;
                        }
                        break;

                    case SECURITY_SENSOR_REED:
                        if (!GpioPinRead(sensor->gpio)) {
                            sensor->detected = true;
                        }
                        break;
                }

                if (timer == SECURITY_SENSOR_TIME_MAX_SEC) {
                    if (sensor->counter >= SECURITY_DETECTED_TIME_MAX_SEC) {
                        sensor->counter = 0;
                        sensor->detected = true;
                    } else {
                        sensor->counter = 0;
                    }
                }

                if (sensor->detected) {
                    LogF(LOG_TYPE_INFO, "SECURITY", "Security sensor \"%s\" detected!", sensor->name);

                    if (sensor->alarm && !Security.alarm) {
                        SecurityAlarmSet(true, true);
                    }

                    StackUnit *unit = StackUnitGet(RPC_DEFAULT_UNIT);
                    snprintf(msg, STR_LEN, "ОХРАНА:%s+Обнаружено+проникновение+%s", unit->name, sensor->name);

                    if (sensor->sms) {
                        if (!NotifierSmsSend(msg)) {
                            Log(LOG_TYPE_ERROR, "SECURITY", "Failed to send sms message");
                        } else {
                            Log(LOG_TYPE_INFO, "SECURITY", "Alarm sms was sended to phone");
                        }
                    }

                    if (sensor->telegram) {
                        if (!NotifierTelegramSend(msg)) {
                            Log(LOG_TYPE_ERROR, "SECURITY", "Failed to send telegram message");
                        } else {
                            Log(LOG_TYPE_INFO, "SECURITY", "Alarm message was sended to telegram");
                        }
                    }
                }
            }
        }

        UtilsSecSleep(1);
    }

    return 0;
}

static int KeysThread(void *data)
{
    GList   *cur_keys = NULL;
    bool    ow_error = false;

    for (;;) {
        if (!OneWireKeysRead(&cur_keys)) {
            if (!ow_error) {
                ow_error = true;
                Log(LOG_TYPE_ERROR, "SECURITY", "Failed to read iButton codes");
            }

            g_list_free(cur_keys);
            cur_keys = NULL;

            UtilsSecSleep(1);
            continue;
        } else {
            if (ow_error) {
                ow_error = false;
                Log(LOG_TYPE_INFO, "SECURITY", "Successfully readed iButton codes");
            }
        }

        if (cur_keys == NULL) {
            UtilsSecSleep(1);
            continue;
        }

        for (GList *k = cur_keys; k != NULL; k = k->next) {
            OneWireData *data = (OneWireData *)k->data;

            if (SecurityKeyCheck(data->value)) {
                if (!SecurityStatusSet(!SecurityStatusGet(), true)) {
                    LogF(LOG_TYPE_ERROR, "SECURITY", "Failed to switch security status by iButton");
                }

                if (SecurityStatusGet()) {
                    if (!ScenarioStart(SCENARIO_OUT_HOME)) {
                        Log(LOG_TYPE_ERROR, "SECURITY", "Failed to start scenario OUT_HOME");
                    }
                } else {
                    if (!ScenarioStart(SCENARIO_IN_HOME)) {
                        Log(LOG_TYPE_ERROR, "SECURITY", "Failed to start scenario IN_HOME");
                    }
                }

                LogF(LOG_TYPE_INFO, "SECURITY", "Detected valid key: \"%s\"", data->value);
                UtilsSecSleep(5);

                break;
            } else {
                LogF(LOG_TYPE_ERROR, "SECURITY", "Invalid security key: \"%s\"", data->value);
            }

            free(data);
        }

         g_list_free(cur_keys);
        cur_keys = NULL;

        UtilsSecSleep(1);
    }

    return 0;
}

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

void SecuritySoundSet(SecuritySound sound, bool status)
{
    Security.sound[sound] = status;
}

void SecurityGpioSet(SecurityGpio id, GpioPin *gpio)
{
    Security.gpio[id] = gpio;
}

bool SecurityKeyCheck(const char *key)
{
    for (GList *ck = Security.keys; ck != NULL; ck = ck->next) {
        SecurityKey *skey = (SecurityKey *)ck->data;

        if (!strcmp(key, skey->id)) {
            return true;
        }
    }

    return false;
}

bool SecurityControllerStart()
{
    thrd_t  sens_th, keys_th, alrm_th;

    Log(LOG_TYPE_INFO, "SECURITY", "Starting Security controller");

    thrd_create(&sens_th, &SensorsThread, NULL);
    thrd_detach(sens_th);
    thrd_create(&keys_th, &KeysThread, NULL);
    thrd_detach(keys_th);
    thrd_create(&alrm_th, &AlarmThread, NULL);
    thrd_detach(alrm_th);

    return true;
}

bool SecurityStatusSet(bool status, bool save)
{
    thrd_t  bzr_th, ntf_th, save_th;

    Security.status = status;

    if (!status) {
        GpioPinWrite(Security.gpio[SECURITY_GPIO_STATUS_LED], false);

        for (GList *s = Security.sensors; s != NULL; s = s->next) {
            SecuritySensor *sensor = (SecuritySensor *)s->data;
            sensor->detected = false;
            sensor->counter = 0;
        }

        LogF(LOG_TYPE_INFO, "SECURITY", "Security controller disabled");

        SecurityAlarmSet(false, true);
    } else {
        GpioPinWrite(Security.gpio[SECURITY_GPIO_STATUS_LED], true);
        LogF(LOG_TYPE_INFO, "SECURITY", "Security controller enabled");
    }

    thrd_create(&bzr_th, &BuzzerThread, NULL);
    thrd_detach(bzr_th);
    thrd_create(&ntf_th, &StatusNotifyThread, NULL);
    thrd_detach(ntf_th);

    if (save) {
        thrd_create(&save_th, &StatusSaveThread, NULL);
        thrd_detach(save_th);
    }

    return true;
}

bool SecurityAlarmSet(bool status, bool save)
{
    Security.alarm = status;

    if (status) {
        GpioPinWrite(Security.gpio[SECURITY_GPIO_ALARM_RELAY], true);
        AlarmHandler();
        LogF(LOG_TYPE_INFO, "SECURITY", "Security controller alarm enabled");
    } else {
        GpioPinWrite(Security.gpio[SECURITY_GPIO_BUZZER], false);
        GpioPinWrite(Security.gpio[SECURITY_GPIO_ALARM_LED], false);
        GpioPinWrite(Security.gpio[SECURITY_GPIO_ALARM_RELAY], false);
        LogF(LOG_TYPE_INFO, "SECURITY", "Security controller alarm disabled");
    }

    if (save) {
        if (!StatusSave(SECURITY_SAVE_TYPE_ALARM, status)) {
            return false;
        }
    }

    return true;
}

bool SecurityAlarmGet()
{
    return Security.alarm;
}

bool SecurityStatusGet()
{
    return Security.status;
}

void SecuritySensorAdd(const SecuritySensor *sensor)
{
    Security.sensors = g_list_append(Security.sensors, (void *)sensor);
}

SecuritySensor *SecuritySensorGet(const char *name)
{
    for (GList *c = Security.sensors; c != NULL; c = c->next) {
        SecuritySensor *sensor = (SecuritySensor *)c->data;
        if (!strcmp(sensor->name, name)) {
            return sensor;
        }
    }
    return NULL;
}

GList **SecuritySensorsGet()
{
    return &Security.sensors;
}

void SecurityKeyAdd(const SecurityKey *key)
{
    Security.keys = g_list_append(Security.keys, (void *)key);
}
