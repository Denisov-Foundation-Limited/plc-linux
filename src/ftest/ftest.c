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

#include <glib-2.0/glib.h>

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
    bool    dev_found = false;
    GList   *devices = NULL;
    bool    state = false;

   LogPrint(LOG_TYPE_INFO, "FTEST", "Starting FTest");

    for (;;) {
        LogPrint(LOG_TYPE_INFO, "FTEST", "=========================================================================");
        LogPrint(LOG_TYPE_INFO, "FTEST", "FACTORY TEST");
        LogPrint(LOG_TYPE_INFO, "FTEST", "");

        /**
         * GPIO test
         */

        LogPrint(LOG_TYPE_INFO, "FTEST", "GPIO TEST:");

        GList **gpios = GpioPinsGet();
        for (GList *p = *gpios; p != NULL; p = p->next) {
            GpioPin *pin = (GpioPin *)p->data;

            if (pin->mode == GPIO_MODE_OUTPUT) {
                continue;
            }

            if (!strcmp(pin->name, "none")) {
                continue;
            }

            switch (pin->type) {
                case GPIO_TYPE_DIGITAL:
                    if (!GpioPinRead(pin, &state)) {
                        LogPrintF(LOG_TYPE_ERROR, "FTEST", "Failed to read GPIO \"%s\"", pin->name);
                        continue;
                    }

                    if (state) {
                        strncpy(gpio_value, "HIGH", SHORT_STR_LEN);
                    } else {
                        strncpy(gpio_value, "LOW", SHORT_STR_LEN);
                    }
                    strncpy(gpio_type, "DIGITAL", SHORT_STR_LEN);
                    break;

                case GPIO_TYPE_ANALOG:
                    if (!GpioPinReadA(pin, &val)) {
                        LogPrintF(LOG_TYPE_ERROR, "FTEST", "Failed to read GPIO \"%s\"", pin->name);
                        continue;
                    }
                    snprintf(gpio_value, SHORT_STR_LEN, "%d", val);
                    strncpy(gpio_type, "ANALOG", SHORT_STR_LEN);
                    break;
            }

           LogPrintF(LOG_TYPE_INFO, "FTEST", "\t\tRead GPIO name: \"%s\"\ttype: \"%s\"\tstate \"%s\"", pin->name, gpio_type, gpio_value);
        }

        last_state = !last_state;

        LogPrint(LOG_TYPE_INFO, "FTEST", "");
        for (GList *p = *gpios; p != NULL; p = p->next) {
            GpioPin *pin = (GpioPin *)p->data;

            if (pin->mode == GPIO_MODE_INPUT || pin->type == GPIO_TYPE_ANALOG) {
                continue;
            }

            if (!strcmp(pin->name, "none")) {
                continue;
            }

            if (last_state) {
                strncpy(gpio_value, "HIGH", SHORT_STR_LEN);
            } else {
                strncpy(gpio_value, "LOW", SHORT_STR_LEN);
            }

            GpioPinWrite(pin, last_state);

            strncpy(gpio_type, "DIGITAL", SHORT_STR_LEN);
            LogPrintF(LOG_TYPE_INFO, "FTEST", "\t\tWrite GPIO name: \"%s\"\ttype: \"%s\"\tstate \"%s\"", pin->name, gpio_type, gpio_value);
        }

        /**
         * LCD test
         */

        LogPrint(LOG_TYPE_INFO, "FTEST", "");
        LogPrint(LOG_TYPE_INFO, "FTEST", "LCD TEST:");

        strncpy(lcd_text[0], "  FACTORY TEST", SHORT_STR_LEN);
        strncpy(lcd_text[1], "  DISPLAY TEST", SHORT_STR_LEN);

        GList **lcds = LcdsGet();
        for (GList *l = *lcds; l != NULL; l = l->next) {
            LCD *lcd = (LCD *)l->data;

            LcdClear(lcd);
            LcdPosSet(lcd, 0, 0);
            LcdPrint(lcd, lcd_text[0]);
            LcdPosSet(lcd, 1, 0);
            LcdPrint(lcd, lcd_text[1]);

           LogPrintF(LOG_TYPE_INFO, "FTEST", "\t\tDisplay name: \"%s\" row[0]: \"%s\" row[1]: \"%s\"", lcd->name, lcd_text[0], lcd_text[1]);
        }

        /**
         * OneWire test
         */

       LogPrint(LOG_TYPE_INFO, "FTEST", "");
       LogPrint(LOG_TYPE_INFO, "FTEST", "1-WIRE TEST:");

       if (!OneWireDevicesList(&devices)) {
           LogPrintF(LOG_TYPE_INFO, "FTEST", "\t\tFailed to read 1-Wire devices list");
        } else {
            if (devices != NULL) {
                for (GList *d = devices; d != NULL; d = d->next) {
                    OneWireData *data = (OneWireData *)d->data;
                    dev_found = true;

                   LogPrintF(LOG_TYPE_INFO, "FTEST", "\t\tDevice detected: \"%s\"", data->value);

                    free(data);
                }
            }
        }

        if (!dev_found) {
           LogPrint(LOG_TYPE_INFO, "FTEST", "\t\tDevices not found");
        }

        g_list_free(devices);
        devices = NULL;

        UtilsSecSleep(1);
    }

    return 0;
}

bool FactoryTestStart()
{
    thrd_t  test_th;
    int     test_res;

    if (thrd_create(&test_th, &TestThread, NULL) != thrd_success) {
        return false;
    }
    if (thrd_join(test_th, &test_res) != thrd_success) {
        return false;
    }

    if (test_res < 0) {
        return false;
    }

    return true;
}
