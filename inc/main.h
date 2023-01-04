/*
*******************************************************************************
 *  Author:             Craig Hemingway                                       *
 *  Company:            Dynament Ltd.                                         *
 *                      Status Scientific Controls Ltd.                       *
 *  Project :           24-Way Premier IR Sensor Jig                          *
 *  Filename:           main.h                                                *
 *  Date:               04/01/2023                                            *
 *  File Version:   	1.0.0                                                 *
 *  Version history:    1.0.0 - 04/01/2022 - Craig Hemingway                  *
 *                          Initial release                                   *
 *  Tools Used: Visual Studio Code -> 1.73.1                                  *
 *              Compiler           -> GCC 11.3.1 arm-none-eabi                *
 *                                                                            *
 ******************************************************************************
*/


// Define to prevent recursive inclusion
#ifndef __MAIN_H
#define __MAIN_H

#include <stdint.h>
#include <stdio.h>
#include <stdbool.h>

#include <hardware/pio_instructions.h>
#include <pico/stdlib.h>

#define NOP                 pio_encode_nop ( )
#define SENSOR_FAIL         0
#define SENSOR_PASS         1
#define SENSOR_CHECKING     2

#define WATCHDOG_MILLISECONDS   8000    // Maximum 8 300 ms

// GPIO
#define BIT_A_PIN        7
#define BIT_B_PIN        8
#define BIT_C_PIN        9
#define BIT_D_PIN       10
#define I2C_SCL_PIN     27
#define I2C_SDA_PIN     26
#define LED_B1_PIN      16
#define LED_B2_PIN      17
#define LED_G1_PIN      14
#define LED_G2_PIN      15
#define LED_PICO_PIN    25
#define LED_R1_PIN      12
#define LED_R2_PIN      13
#define MATRIX_CLK_PIN   5
#define MATRIX_LAT_PIN   4
#define MATRIX_OE_PIN    6
#define SPI_CS_PIN       1
#define SPI_MISO_PIN     0
#define SPI_MOSI_PIN     3
#define SPI_SCK_PIN      2
#define SW1             21
#define SW2             20
#define SW3             19
#define SW4             18

#define BIT_A_HIGH          gpio_put ( BIT_A_PIN      , 1 )
#define BIT_A_LOW           gpio_put ( BIT_A_PIN      , 0 )
#define BIT_B_HIGH          gpio_put ( BIT_B_PIN      , 1 )
#define BIT_B_LOW           gpio_put ( BIT_B_PIN      , 0 )
#define BIT_C_HIGH          gpio_put ( BIT_C_PIN      , 1 )
#define BIT_C_LOW           gpio_put ( BIT_C_PIN      , 0 )
#define BIT_D_HIGH          gpio_put ( BIT_D_PIN      , 1 )
#define BIT_D_LOW           gpio_put ( BIT_D_PIN      , 0 )
#define LED_B1_HIGH         gpio_put ( LED_B1_PIN     , 1 )
#define LED_B1_LOW          gpio_put ( LED_B1_PIN     , 0 )
#define LED_B2_HIGH         gpio_put ( LED_B2_PIN     , 1 )
#define LED_B2_LOW          gpio_put ( LED_B2_PIN     , 0 )
#define LED_G1_HIGH         gpio_put ( LED_G1_PIN     , 1 )
#define LED_G1_LOW          gpio_put ( LED_G1_PIN     , 0 )
#define LED_G2_HIGH         gpio_put ( LED_G2_PIN     , 1 )
#define LED_G2_LOW          gpio_put ( LED_G2_PIN     , 0 )
#define LED_PICO_OFF        gpio_put ( LED_PICO_PIN   , 0 )
#define LED_PICO_ON         gpio_put ( LED_PICO_PIN   , 1 )
#define LED_R1_HIGH         gpio_put ( LED_R1_PIN     , 1 )
#define LED_R1_LOW          gpio_put ( LED_R1_PIN     , 0 )
#define LED_R2_HIGH         gpio_put ( LED_R2_PIN     , 1 )
#define LED_R2_LOW          gpio_put ( LED_R2_PIN     , 0 )
#define MATRIX_CLK_HIGH     gpio_put ( MATRIX_CLK_PIN , 1 )
#define MATRIX_CLK_LOW      gpio_put ( MATRIX_CLK_PIN , 0 )
#define MATRIX_LAT_HIGH     gpio_put ( MATRIX_LAT_PIN , 1 )
#define MATRIX_LAT_LOW      gpio_put ( MATRIX_LAT_PIN , 0 )
#define MATRIX_OUTPUT_OFF   gpio_put ( MATRIX_OE_PIN  , 1 )
#define MATRIX_OUTPUT_ON    gpio_put ( MATRIX_OE_PIN  , 0 )

#define BIT_A_MASK          0b0000000000000001
#define BIT_B_MASK          0b0000000000000010
#define BIT_C_MASK          0b0000000000000100
#define BIT_D_MASK          0b0000000000001000
#define LED_BLUE_TOP        0b0000000000010000
#define LED_BLUE_BOTTOM     0b0000000010000000
#define LED_GREEN_TOP       0b0000000000100000
#define LED_GREEN_BOTTOM    0b0000000100000000
#define LED_RED_TOP         0b0000000001000000
#define LED_RED_BOTTOM      0b0000001000000000

// SPI
#define SPI_BAUD_RATE       100 // kHz
#define SPI_BUFFER_LENGTH   10
#define SPI_CS_HIGH         gpio_put ( SPI_CS_PIN , 1 )
#define SPI_CS_LOW          gpio_put ( SPI_CS_PIN , 0 )
#define SPI_MASTER          spi0

#endif /* __MAIN_H */

/*** end of file ***/
