/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#include <controllers/controllers.h>
#include <controllers/security.h>
#include <controllers/meteo.h>
#include <utils/log.h>
#include <controllers/socket.h>
#include <controllers/tank.h>

/*********************************************************************/
/*                                                                   */
/*                          PUBLIC FUNCTIONS                         */
/*                                                                   */
/*********************************************************************/

bool ControllersStart()
{
    if (!SecurityControllerStart()) {
        Log(LOG_TYPE_ERROR, "CONTROLLERS", "Failed to start Security controller");
        return false;
    }

    if (!MeteoControllerStart()) {
        Log(LOG_TYPE_ERROR, "CONTROLLERS", "Failed to start Meteo controller");
        return false;
    }

    if (!SocketControllerStart()) {
        Log(LOG_TYPE_ERROR, "CONTROLLERS", "Failed to start Socket controller");
        return false;
    }

    if (!TankControllerStart()) {
        Log(LOG_TYPE_ERROR, "CONTROLLERS", "Failed to start Tank controller");
        return false;
    }

    return true;
}
