/*
*******************************************************************************
 *  Author:             Craig Hemingway                                       *
 *  Company:            Dynament Ltd.                                         *
 *                      Status Scientific Controls Ltd.                       *
 *  Project :           24-Way Premier IR Sensor Jig                          *
 *  Filename:           matrix_default.h                                      *
 *  Date:               08/12/2022                                            *
 *  File Version:   	1.0.0                                                 *
 *  Version history:    1.0.0 - 08/12/2022 - Craig Hemingway                  *
 *                          Initial release                                   *
 *  Tools Used: Visual Studio Code -> 1.73.1                                  *
 *              Compiler           -> GCC 11.3.1 arm-none-eabi                *
 *                                                                            *
 ******************************************************************************
*/

// Define to prevent recursive inclusion
#ifndef __MATRIX_DEFAULT_H
#define __MATRIX_DEFAULT_H

#include <stdint.h>

// Top rows ( RGB , 0 to 15 ) , Bottom rows ( RGB , 16 to 31 ) , ( MSB ) BIT_D , BIT_C , BIT_B , BIT_A ( LSB )

const uint16_t MatrixRow [ ] = {
    0b00000000, // Row 0
    0b00000001, // Row 1
    0b00000010, // Row 2
    0b00000011, // Row 3
    0b00000100, // Row 4
    0b00000101, // Row 5
    0b00000110, // Row 6
    0b00000111, // Row 7
    0b00001000, // Row 8
    0b00001001, // Row 9
    0b00001010, // Row 10
    0b00001011, // Row 11
    0b00001100, // Row 12
    0b00001101, // Row 13
    0b00001110, // Row 14
    0b00001111, // Row 15
    0b00000000, // Row 16
    0b00000001, // Row 17
    0b00000010, // Row 18
    0b00000011, // Row 19
    0b00000100, // Row 20
    0b00000101, // Row 21
    0b00000110, // Row 22
    0b00000111, // Row 23
    0b00001000, // Row 24
    0b00001001, // Row 25
    0b00001010, // Row 26
    0b00001011, // Row 27
    0b00001100, // Row 28
    0b00001101, // Row 29
    0b00001110, // Row 30
    0b00001111, // Row 31
};

#endif /* __MATRIX_DEFAULT_H */

/*** end of file ***/
