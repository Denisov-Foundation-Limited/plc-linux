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
#include <plc/plc.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE VARIABLES                         */
/*                                                                   */
/*********************************************************************/

static struct _Security {
    GList           *sensors;
    GList           *keys;
    GpioPin         *gpio[SECURITY_GPIO_MAX];
    mtx_t           sts_mtx;
    bool            status;
    bool            alarm;
    bool            last_alarm;
    bool            sound[SECURITY_SOUND_MAX];
    bool            enabled;
} Security = {
    .sensors = NULL,
    .keys = NULL,
    .status = false,
    .alarm = false,
    .last_alarm = false,
    .enabled = false
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

static int SensorsThread(void *data)
{
    char        msg[STR_LEN];
    unsigned    timer = 0;
    bool        state = false;

    for (;;) {
        timer++;

        if (timer > SECURITY_SENSOR_TIME_MAX_SEC) {
            timer = 0;
        }

        for (GList *s = Security.sensors; s != NULL; s = s->next) {
            SecuritySensor *sensor = (SecuritySensor *)s->data;

            if (sensor->detected) {
                continue;
            }

            switch (sensor->type) {
                case SECURITY_SENSOR_MICRO_WAVE:
                    if (!GpioPinRead(sensor->gpio, &state)) {
                        LogF(LOG_TYPE_ERROR, "SECURITY", "Failed to read GPIO \"%s\"", sensor->gpio->name);
                        break;
                    }

                    if (!state) {
                        sensor->counter++;
                    }
                    break;

                case SECURITY_SENSOR_PIR:
                    if (!GpioPinRead(sensor->gpio, &state)) {
                        LogF(LOG_TYPE_ERROR, "SECURITY", "Failed to read GPIO \"%s\"", sensor->gpio->name);
                        break;
                    }

                    if (state) {
                        sensor->counter++;
                    }
                    break;

                case SECURITY_SENSOR_REED:
                    if (!GpioPinRead(sensor->gpio, &state)) {
                        LogF(LOG_TYPE_ERROR, "SECURITY", "Failed to read GPIO \"%s\"", sensor->gpio->name);
                        break;
                    }

                    if (!state) {
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

            mtx_lock(&Security.sts_mtx);
            if (sensor->detected && Security.status) {
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
            mtx_unlock(&Security.sts_mtx);
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

bool SecurityEnabledGet()
{
    return Security.enabled;
}

void SecurityEnabledSet(bool state)
{
    Security.enabled = state;
}

SecuritySensor *SecuritySensorNew(const char *name, SecuritySensorType type, GpioPin *gpio, bool telegram, bool sms, bool alarm)
{
    SecuritySensor *sensor = (SecuritySensor *)malloc(sizeof(SecuritySensor));

    strncpy(sensor->name, name, SHORT_STR_LEN);
    sensor->type = type;
    sensor->gpio = gpio;
    sensor->telegram = telegram;
    sensor->sms = sms;
    sensor->alarm = alarm;
    sensor->counter = 0;
    sensor->detected = false;

    return sensor;
}

SecurityKey *SecurityKeyNew(const char *name, const char *value)
{
    SecurityKey *key = (SecurityKey *)malloc(sizeof(SecurityKey));

    strncpy(key->name, name, SHORT_STR_LEN);
    strncpy(key->id, value, SHORT_STR_LEN);

    return key;
}

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
    thrd_t  sens_th, keys_th;

    if (!SecurityEnabledGet()) {
        return true;
    }

    Log(LOG_TYPE_INFO, "SECURITY", "Starting Security controller");

    if (thrd_create(&sens_th, &SensorsThread, NULL) != thrd_success) {
        return false;
    }
    if (thrd_detach(sens_th) != thrd_success) {
        return false;
    }
    if (thrd_create(&keys_th, &KeysThread, NULL) != thrd_success) {
        return false;
    }
    if (thrd_detach(keys_th)) {
        return false;
    }

    return true;
}

bool SecurityStatusSet(bool status, bool save)
{
    char    msg[STR_LEN];

    if (status != Security.status) {
        mtx_lock(&Security.sts_mtx);

        Security.status = status;

        if (!status) {
            LogF(LOG_TYPE_INFO, "SECURITY", "Security controller disabled");
            SecurityAlarmSet(false, true);
        } else {
            LogF(LOG_TYPE_INFO, "SECURITY", "Security controller enabled");
        }

        for (GList *s = Security.sensors; s != NULL; s = s->next) {
            SecuritySensor *sensor = (SecuritySensor *)s->data;
            sensor->detected = false;
            sensor->counter = 0;
        }

        GpioPinWrite(Security.gpio[SECURITY_GPIO_STATUS_LED], status);

        /**
         * Buzzer on/off
         */

        if (status && Security.sound[SECURITY_SOUND_EXIT]) {
            PlcBuzzerRun(PLC_BUZZER_SECURITY_EXIT, true);
        } else if (!status && Security.sound[SECURITY_SOUND_ENTER]) {
            PlcBuzzerRun(PLC_BUZZER_SECURITY_ENTER, true);
        }

        /**
         * Save status to DB
         */

        if (save) {
            StatusSave(SECURITY_SAVE_TYPE_STATUS, status);
        }

        /**
         * Send notify
         */

        StackUnit *unit = StackUnitGet(RPC_DEFAULT_UNIT);

        if (status) {
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

        mtx_unlock(&Security.sts_mtx);
    }

    return true;
}

bool SecurityAlarmSet(bool status, bool save)
{
    Security.alarm = status;

    if (status) {
        PlcAlarmSet(PLC_ALARM_SECURITY, true);
        if (Security.sound[SECURITY_SOUND_ALARM]) {
            PlcBuzzerRun(PLC_BUZZER_LOOP, true);
        }
        GpioPinWrite(Security.gpio[SECURITY_GPIO_ALARM_RELAY], true);
        LogF(LOG_TYPE_INFO, "SECURITY", "Security controller alarm enabled");
    } else {
        PlcAlarmSet(PLC_ALARM_SECURITY, false);
        if (Security.sound[SECURITY_SOUND_ALARM]) {
            PlcBuzzerRun(PLC_BUZZER_LOOP, false);
        }
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
