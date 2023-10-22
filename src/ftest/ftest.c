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

#include <glib.h>

#include <ftest/ftest.h>
#include <utils/log.h>
#include <core/gpio.h>
#include <core/lcd.h>
#include <core/extenders.h>
#include <core/onewire.h>
#include <utils/utils.h>

/*********************************************************************/
/*                                                                   */
/*                         PRIVATE FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

static int TestThread(void *data)
{
    char    gpio_value[SHORT_STR_LEN];
    char    gpio_type[SHORT_STR_LEN];
    char    lcd_text[2][SHORT_STR_LEN];
    int     val;
    bool    last_state = false;
    GList   **devices = NULL;

    Log(LOG_TYPE_INFO, "FTEST", "Starting FTest");

    for (;;) {
        Log(LOG_TYPE_INFO, "FTEST", "=========================================================================");
        Log(LOG_TYPE_INFO, "FTEST", "FACTORY TEST");
        Log(LOG_TYPE_INFO, "FTEST", "");

        /**
         * GPIO test
         */

        Log(LOG_TYPE_INFO, "FTEST", "GPIO TEST:");

        GList **gpios = GpioPinsGet();
        for (GList *p = *gpios; p != NULL; p = p->next) {
            GpioPin *pin = (GpioPin *)p->data;

            if (pin->mode == GPIO_MODE_OUTPUT) {
                continue;
            }

            switch (pin->type) {
                case GPIO_TYPE_DIGITAL:
                    if (GpioPinRead(pin)) {
                        strncpy(gpio_value, "HIGH", SHORT_STR_LEN);
                    } else {
                        strncpy(gpio_value, "LOW", SHORT_STR_LEN);
                    }
                    strncpy(gpio_type, "DIGITAL", SHORT_STR_LEN);
                    break;

                case GPIO_TYPE_ANALOG:
                    val = GpioPinReadA(pin);
                    snprintf(gpio_value, STR_LEN, "%d", val);
                    strncpy(gpio_type, "ANALOG", SHORT_STR_LEN);
                    break;
            }

            LogF(LOG_TYPE_INFO, "FTEST", "\t\tRead GPIO name: \"%s\"\ttype: \"%s\"\tstate \"%s\"", pin->name, gpio_type, gpio_value);
        }

        last_state = !last_state;

        Log(LOG_TYPE_INFO, "FTEST", "");
        for (GList *p = *gpios; p != NULL; p = p->next) {
            GpioPin *pin = (GpioPin *)p->data;

            if (pin->mode == GPIO_MODE_INPUT || pin->type == GPIO_TYPE_ANALOG) {
                continue;
            }

            if (last_state) {
                strncpy(gpio_value, "HIGH", SHORT_STR_LEN);
            } else {
                strncpy(gpio_value, "LOW", SHORT_STR_LEN);
            }

            if (!GpioPinWrite(pin, last_state)) {
                strncpy(gpio_value, "FAIL", SHORT_STR_LEN);
            }

            strncpy(gpio_type, "DIGITAL", SHORT_STR_LEN);
            LogF(LOG_TYPE_INFO, "FTEST", "\t\tWrite GPIO name: \"%s\"\ttype: \"%s\"\tstate \"%s\"", pin->name, gpio_type, gpio_value);
        }

        /**
         * LCD test
         */

        Log(LOG_TYPE_INFO, "FTEST", "");
        Log(LOG_TYPE_INFO, "FTEST", "LCD TEST:");

        strncpy(lcd_text[0], "  FACTORY TEST", SHORT_STR_LEN);
        strncpy(lcd_text[1], "  DISPLAY TEST", SHORT_STR_LEN);

        GList **lcds = LcdsGet();
        for (GList *l = *lcds; l != NULL; l = l->next) {
            LCD *lcd = (LCD *)l->data;

            if (!LcdClear(lcd)) {
                LogF(LOG_TYPE_INFO, "FTEST", "\t\tDisplay name: \"%s\" Failed to clear LCD", lcd->name);
            }
            if (!LcdPosSet(lcd, 0, 0)) {
                LogF(LOG_TYPE_INFO, "FTEST", "\t\tDisplay name: \"%s\" Failed to set pos LCD", lcd->name);
            }
            if (!LcdPrint(lcd, lcd_text[0])) {
                LogF(LOG_TYPE_INFO, "FTEST", "\t\tDisplay name: \"%s\" Failed to print text[0] LCD", lcd->name);
            }
            if (!LcdPosSet(lcd, 1, 0)) {
                LogF(LOG_TYPE_INFO, "FTEST", "\t\tDisplay name: \"%s\" Failed to set pos LCD", lcd->name);
            }
            if (!LcdPrint(lcd, lcd_text[1])) {
                LogF(LOG_TYPE_INFO, "FTEST", "\t\tDisplay name: \"%s\" Failed to print text[1] LCD", lcd->name);
            }

            LogF(LOG_TYPE_INFO, "FTEST", "\t\tDisplay name: \"%s\" row[0]: \"%s\" row[1]: \"%s\"", lcd->name, lcd_text[0], lcd_text[1]);
        }

        /**
         * OneWire test
         */

        Log(LOG_TYPE_INFO, "FTEST", "");
        Log(LOG_TYPE_INFO, "FTEST", "1-WIRE TEST:");

        if (!OneWireDevicesList(devices)) {
            LogF(LOG_TYPE_INFO, "FTEST", "\t\tFailed to read 1-Wire devices list");
        } else {
            for (GList *d = *devices; d != NULL; d = d->next) {
                OneWireData *data = (OneWireData *)d->data;

                LogF(LOG_TYPE_INFO, "FTEST", "\t\tDevice detected: \"%s\"", data->value);

                if (data != NULL) {
                    free(data);
                }
            }
        }

        if (devices != NULL) {
            g_list_free(*devices);
        }

        thrd_sleep(&(struct timespec){ .tv_sec = 1 }, NULL);
    }

    return 0;
}

bool FactoryTestStart()
{
    thrd_t  test_th;
    int     test_res;

    thrd_create(&test_th, &TestThread, NULL);
    thrd_join(test_th, &test_res);

    if (test_res < 0) {
        return false;
    }

    return true;
}
