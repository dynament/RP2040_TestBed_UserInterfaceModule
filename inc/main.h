/*
*******************************************************************************
 *  Author:             Craig Hemingway                                       *
 *  Company:            Dynament Ltd.                                         *
 *                      Status Scientific Controls Ltd.                       *
 *  Project :           24-Way Premier IR Sensor Jig                          *
 *  Filename:   		main.h                                                *
 *  Date:		        06/12/2022                                            *
 *  File Version:   	1.0.0                                                 *
 *  Version history:    1.0.0 - 06/12/2022 - Craig Hemingway                  *
 *                          Initial release                                   *
 *  Tools Used: Visual Studio Code -> 1.73.1                                  *
 *              Compiler           -> GCC 11.3.1 arm-none-eabi                *
 *                                                                            *
 ******************************************************************************
*/


/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __MAIN_H
#define __MAIN_H

#include <hardware/pio_instructions.h>
#include <pico/stdlib.h>

#define NOP     pio_encode_nop ( )
#define WATCHDOG_MILLISECONDS   8000    // Maximum 8 300 ms

// GPIO
#define LED_PICO_PIN    25

#define LED_PICO_OFF    gpio_put ( LED_PICO_PIN   , 0 )
#define LED_PICO_ON     gpio_put ( LED_PICO_PIN   , 1 )

#endif /* __MAIN_H */

/*** end of file ***/
