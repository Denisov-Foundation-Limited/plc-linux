/*********************************************************************/
/*                                                                   */
/* Future City Programmable Logic Controller                         */
/*                                                                   */
/* Copyright (C) 2023 Denisov Smart Devices Limited                  */
/* License: GPLv3                                                    */
/* Written by Sergey Denisov aka LittleBuster (DenisovS21@gmail.com) */
/*                                                                   */
/*********************************************************************/

#ifndef __FTEST_H__
#define __FTEST_H__

#include <stdbool.h>

/**
 * @brief Starting Factory Test
 * 
 * @param input Test only inputs
 *
 * @return Result of FTtest
 */
bool FactoryTestStart(bool input);

#endif /* __FTEST_H__ */
