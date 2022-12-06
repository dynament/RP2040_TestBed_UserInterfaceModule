/*
*******************************************************************************
 *  Author:             Craig Hemingway                                       *
 *  Company:            Dynament Ltd.                                         *
 *                      Status Scientific Controls Ltd.                       *
 *  Project :           24-Way Premier IR Sensor Jig                          *
 *  Filename:   		main.c                                                *
 *  Date:		        06/12/2022                                            *
 *  File Version:   	1.0.0                                                 *
 *  Version history:    1.0.0 - 06/12/2022 - Craig Hemingway                  *
 *                          Initial release                                   *
 *  Tools Used: Visual Studio Code -> 1.73.1                                  *
 *              Compiler           -> GCC 11.3.1 arm-none-eabi                *
 *                                                                            *
 ******************************************************************************
*/

#include <main.h>

#include <hardware/watchdog.h>
#include <pico/binary_info.h>

struct repeating_timer timer_heartbeat;

void watchdog ( void );

// Timer interrupts
bool timer_heartbeat_500ms_callback ( struct repeating_timer *t )
{
    if ( gpio_get ( LED_PICO_PIN ) )
    {
        LED_PICO_OFF;
    }
    else
    {
        LED_PICO_ON;
    }
}

void main ( void )
{
    // Useful information for picotool
    bi_decl ( bi_program_description ( "RP2040 Premier" ) );

    // Initialise standard stdio types
    stdio_init_all ( );

    // Set up watchdog
    watchdog_enable ( WATCHDOG_MILLISECONDS , 1 );

    // Initialize all configured peripherals
    // Set up GPIO
    gpio_init    ( LED_PICO_PIN   );
    gpio_set_dir ( LED_PICO_PIN   , GPIO_OUT );

    LED_PICO_OFF;

    // Set up timer interrupts
    add_repeating_timer_ms ( 500 , timer_heartbeat_500ms_callback , NULL , &timer_heartbeat );

    // Infinite loop
    for ( ; ; )
    {
        watchdog ( );
    }
}

void watchdog ( void )
{
    watchdog_update ( );
}

/*** end of file ***/
