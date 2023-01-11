/*
*******************************************************************************
 *  Author:             Craig Hemingway                                       *
 *  Company:            Dynament Ltd.                                         *
 *                      Status Scientific Controls Ltd.                       *
 *  Project :           24-Way Premier IR Sensor Jig                          *
 *  Filename:           main.c                                                *
 *  Date:               04/01/2023                                            *
 *  File Version:   	1.0.0                                                 *
 *  Version history:    1.0.0 - 04/01/2023 - Craig Hemingway                  *
 *                          Initial release                                   *
 *  Tools Used: Visual Studio Code -> 1.73.1                                  *
 *              Compiler           -> GCC 11.3.1 arm-none-eabi                *
 *                                                                            *
 ******************************************************************************
*/

#include <main.h>
#include <matrix_default.h>

#include <string.h>
#include <hardware/spi.h>
#include <hardware/watchdog.h>
#include <pico/binary_info.h>

// LED Matrix
#define MATRIX_HEIGHT   32
#define MATRIX_WIDTH    32
const uint64_t MATRIX_DELAY_REFRESH = 450;  // microseconds
uint16_t MatrixData [ MATRIX_HEIGHT ] [ MATRIX_WIDTH ];

// SPI
uint8_t SPI_RxBuffer [ 11 ] = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 };
const    uint16_t SPI_RX_PERIOD = 500;  // Minimum delay ( ms ) between messages ( polling )
const    uint16_t SPI_TX_PERIOD = 500;  // Minimum delay ( ms ) between messages ( button press )
volatile uint16_t g_SPI_RxPeriod  = 0;
volatile uint16_t g_SPI_TxPeriod  = 0;

// DAC check
const uint8_t DAC_CHECK_IS_READY    = 0x10;
const uint8_t DAC_CHECK_RUNNING     = 0x0B;
const uint8_t DAC_CHECK_NOT_RUNNING = 0x0F;
const uint8_t SYNC_BYTE             = 0x55;

// struct repeating_timer refresh_display;
struct repeating_timer timer_1ms;
struct repeating_timer timer_heartbeat;

void DrawMatrix       ( void );
void SetMatrix_Buffer ( uint8_t sensor , uint8_t state , uint8_t pos );
void watchdog         ( void );

// Timer interrupts
bool timer_1ms_callback ( struct repeating_timer *t )
{
    if ( g_SPI_RxPeriod )
    {
        g_SPI_RxPeriod--;
    }
    else
    {
        // Nothing to do
    }

    if ( g_SPI_TxPeriod )
    {
        g_SPI_TxPeriod--;
    }
    else
    {
        // Nothing to do
    }
}

bool timer_heartbeat_callback ( struct repeating_timer *t )
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

int main ( void )
{
    uint8_t SPI_TxBuffer [ 11 ] = { 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 , 0 };

    uint8_t  ButtonPress    = 0;
    uint8_t  DAC_CheckState = 0;
    uint8_t  SensorState    = 0;
    uint8_t  SensorPos      = 0;
    uint32_t SensorPass     = 0;

    volatile uint8_t Counter_Columns = 0;
    volatile uint8_t Counter_Rows    = 0;

    // Useful information for picotool
    bi_decl ( bi_program_description ( "RP2040 Premier" ) );

    // Initialise standard stdio types
    stdio_init_all ( );

    // Set up watchdog
    watchdog_enable ( WATCHDOG_MILLISECONDS , 1 );

    // Initialize all configured peripherals
    // Set up GPIO
    gpio_init    ( BIT_A_PIN      );
    gpio_init    ( BIT_B_PIN      );
    gpio_init    ( BIT_C_PIN      );
    gpio_init    ( BIT_D_PIN      );
    gpio_init    ( LED_B1_PIN     );
    gpio_init    ( LED_B2_PIN     );
    gpio_init    ( LED_G1_PIN     );
    gpio_init    ( LED_G2_PIN     );
    gpio_init    ( LED_PICO_PIN   );
    gpio_init    ( LED_R1_PIN     );
    gpio_init    ( LED_R2_PIN     );
    gpio_init    ( MATRIX_CLK_PIN );
    gpio_init    ( MATRIX_LAT_PIN );
    gpio_init    ( MATRIX_OE_PIN  );
    gpio_init    ( SW1            );
    gpio_init    ( SW2            );
    gpio_init    ( SW3            );
    gpio_init    ( SW4            );
    gpio_set_dir ( BIT_A_PIN      , GPIO_OUT );
    gpio_set_dir ( BIT_B_PIN      , GPIO_OUT );
    gpio_set_dir ( BIT_C_PIN      , GPIO_OUT );
    gpio_set_dir ( BIT_D_PIN      , GPIO_OUT );
    gpio_set_dir ( LED_B1_PIN     , GPIO_OUT );
    gpio_set_dir ( LED_B2_PIN     , GPIO_OUT );
    gpio_set_dir ( LED_G1_PIN     , GPIO_OUT );
    gpio_set_dir ( LED_G2_PIN     , GPIO_OUT );
    gpio_set_dir ( LED_PICO_PIN   , GPIO_OUT );
    gpio_set_dir ( LED_R1_PIN     , GPIO_OUT );
    gpio_set_dir ( LED_R2_PIN     , GPIO_OUT );
    gpio_set_dir ( MATRIX_CLK_PIN , GPIO_OUT );
    gpio_set_dir ( MATRIX_LAT_PIN , GPIO_OUT );
    gpio_set_dir ( MATRIX_OE_PIN  , GPIO_OUT );
    gpio_set_dir ( SW1            , GPIO_IN  );
    gpio_set_dir ( SW2            , GPIO_IN  );
    gpio_set_dir ( SW3            , GPIO_IN  );
    gpio_set_dir ( SW4            , GPIO_IN  );

    // SPI ( Master )
    spi_init          ( SPI_MASTER   , SPI_BAUD_RATE * 1000 );
    spi_set_slave     ( SPI_MASTER   , false                );
    gpio_set_function ( SPI_CS_PIN   , GPIO_FUNC_SPI        );
    gpio_set_function ( SPI_MISO_PIN , GPIO_FUNC_SPI        );
    gpio_set_function ( SPI_MOSI_PIN , GPIO_FUNC_SPI        );
    gpio_set_function ( SPI_SCK_PIN  , GPIO_FUNC_SPI        );

    // Set up timer interrupts
    add_repeating_timer_ms ( 500 , timer_heartbeat_callback , NULL , &timer_heartbeat );
    add_repeating_timer_ms (   1 , timer_1ms_callback       , NULL , &timer_1ms       );

    BIT_A_LOW;
    BIT_B_LOW;
    BIT_C_LOW;
    BIT_D_LOW;
    LED_B1_LOW;
    LED_B2_LOW;
    LED_G1_LOW;
    LED_G2_LOW;
    LED_PICO_OFF;
    LED_R1_LOW;
    LED_R2_LOW;

    // Clear any shift register data
    for ( Counter_Rows = 0 ; Counter_Rows < 32 ; Counter_Rows++ )
    {
        for ( Counter_Columns = 0 ; Counter_Columns < 32 ; Counter_Columns++ )
        {
            MATRIX_CLK_HIGH;
            sleep_us ( 1 );
            MATRIX_CLK_LOW;
            sleep_us ( 1 );
        }
    }

    // Load default pixel data
    for ( Counter_Rows = 0 ; Counter_Rows < MATRIX_HEIGHT ; Counter_Rows++ )
    {
        for ( Counter_Columns = 0 ; Counter_Columns < MATRIX_WIDTH ; Counter_Columns++ )
        {
            if ( Counter_Rows < 16 )
            {
                MatrixData [ Counter_Rows ] [ Counter_Columns ] = MatrixRow [ Counter_Rows ] + 0b0000000000010000;
            }
            else
            {
                MatrixData [ Counter_Rows ] [ Counter_Columns ] = MatrixRow [ Counter_Rows ] + 0b0000000010000000;
            }
        }
    }
    
    MATRIX_CLK_LOW;
    MATRIX_LAT_LOW;
    MATRIX_OUTPUT_OFF;

    // Infinite loop
    for ( ; ; )
    {
        watchdog ( );

        // Get button presses
        ButtonPress = ( ( gpio_get ( SW4 ) ) << 3 ) + ( ( gpio_get ( SW3 ) ) << 2 ) + ( ( gpio_get ( SW2 ) ) << 1 ) + gpio_get ( SW1 );

        if ( !g_SPI_TxPeriod && ( 0 != ButtonPress ) )  // Send command to test bed
        {
            memset ( SPI_RxBuffer , 0 , sizeof ( SPI_RxBuffer ) );

            SPI_TxBuffer [ 0 ] = SYNC_BYTE;
            SPI_TxBuffer [ 1 ] = ButtonPress;

            spi_write_blocking ( SPI_MASTER , SPI_TxBuffer , 2 );

            g_SPI_TxPeriod = SPI_TX_PERIOD;
        }
        else if ( !g_SPI_RxPeriod && ( 0 == ButtonPress ) ) // Poll for data
        {
            memset ( SPI_RxBuffer , 0 , sizeof ( SPI_RxBuffer ) );
            memset ( SPI_TxBuffer , 0 , sizeof ( SPI_TxBuffer ) );

            SPI_TxBuffer [ 0 ] = SYNC_BYTE;
            SPI_TxBuffer [ 1 ] = DAC_CHECK_IS_READY;

            spi_write_read_blocking ( SPI_MASTER , SPI_TxBuffer , SPI_RxBuffer , 11 );

            g_SPI_RxPeriod = SPI_RX_PERIOD;

            if ( ( SYNC_BYTE == SPI_RxBuffer [ 4 ] ) && ( DAC_CHECK_IS_READY == SPI_RxBuffer [ 5 ] ) )
            {
                DAC_CheckState = SPI_RxBuffer [ 6 ];
                SensorPass = ( uint32_t ) ( ( SPI_RxBuffer [ 7 ] << 16 ) + ( SPI_RxBuffer [ 8 ] << 8 ) + SPI_RxBuffer [ 9 ] );
                SensorPos  = SPI_RxBuffer [ 10 ];
            }
            else
            {
                // Nothing to do
            }
        }
        else
        {
            // Nothing to do
        }

        for ( Counter_Columns = 0 ; Counter_Columns < 24 ; Counter_Columns++ )
        {
            SensorState = ( SensorPass >> Counter_Columns ) & 0b00000001; 
            SetMatrix_Buffer ( Counter_Columns , SensorState , SensorPos );
        }

        DrawMatrix ( );
    }
}

void DrawMatrix ( void )
{
    volatile uint8_t Counter_Column = 0;
    volatile uint8_t Counter_Row    = 0;

    for ( Counter_Row = 0 ; Counter_Row < MATRIX_HEIGHT ; Counter_Row++ )
    {
        MATRIX_OUTPUT_OFF;
        MATRIX_LAT_HIGH;

        for ( Counter_Column = 0 ; Counter_Column < MATRIX_WIDTH ; Counter_Column++ )
        {
            gpio_put ( LED_R1_PIN , ( MatrixData [ Counter_Row ] [ Counter_Column ] & LED_RED_TOP      ) );
            gpio_put ( LED_G1_PIN , ( MatrixData [ Counter_Row ] [ Counter_Column ] & LED_GREEN_TOP    ) );
            gpio_put ( LED_B1_PIN , ( MatrixData [ Counter_Row ] [ Counter_Column ] & LED_BLUE_TOP     ) );
            gpio_put ( LED_R2_PIN , ( MatrixData [ Counter_Row ] [ Counter_Column ] & LED_RED_BOTTOM   ) );
            gpio_put ( LED_G2_PIN , ( MatrixData [ Counter_Row ] [ Counter_Column ] & LED_GREEN_BOTTOM ) );
            gpio_put ( LED_B2_PIN , ( MatrixData [ Counter_Row ] [ Counter_Column ] & LED_BLUE_BOTTOM  ) );

            gpio_put ( BIT_D_PIN  , ( MatrixData [ Counter_Row ] [ Counter_Column ] & BIT_D_MASK ) );
            gpio_put ( BIT_C_PIN  , ( MatrixData [ Counter_Row ] [ Counter_Column ] & BIT_C_MASK ) );
            gpio_put ( BIT_B_PIN  , ( MatrixData [ Counter_Row ] [ Counter_Column ] & BIT_B_MASK ) );
            gpio_put ( BIT_A_PIN  , ( MatrixData [ Counter_Row ] [ Counter_Column ] & BIT_A_MASK ) );

            MATRIX_CLK_HIGH;
            sleep_us ( 1 );
            MATRIX_CLK_LOW;
        }

        MATRIX_LAT_LOW;
        MATRIX_OUTPUT_ON;
        sleep_us ( MATRIX_DELAY_REFRESH );
    }

}

void SetMatrix_Buffer ( uint8_t sensor , uint8_t state , uint8_t pos )
{
    volatile uint8_t Counter = 0;

    switch ( sensor )
    {
        case 0:
            for ( Counter = 2 ; Counter < 6 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 0 != pos ) )
                {
                    MatrixData [ Counter ] [ 2 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 0 != pos ) )
                {
                    MatrixData [ Counter ] [ 2 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 2 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 1:
            for ( Counter = 2 ; Counter < 6 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 1 <= pos ) )
                {
                    MatrixData [ Counter ] [ 7 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 1 <= pos ) )
                {
                    MatrixData [ Counter ] [ 7 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 7 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 2:
            for ( Counter = 2 ; Counter < 6 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 2 <= pos ) )
                {
                    MatrixData [ Counter ] [ 12 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 2 <= pos ) )
                {
                    MatrixData [ Counter ] [ 12 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 12 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 3:
            for ( Counter = 2 ; Counter < 6 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 3 <= pos ) )
                {
                    MatrixData [ Counter ] [ 17 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 3 <= pos ) )
                {
                    MatrixData [ Counter ] [ 17 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 17 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 4:
            for ( Counter = 2 ; Counter < 6 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 4 <= pos ) )
                {
                    MatrixData [ Counter ] [ 22 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 4 <= pos ) )
                {
                    MatrixData [ Counter ] [ 22 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 22 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 5:
            for ( Counter = 2 ; Counter < 6 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 5 <= pos ) )
                {
                    MatrixData [ Counter ] [ 27 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 5 <= pos ) )
                {
                    MatrixData [ Counter ] [ 27 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 27 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 6:
            for ( Counter = 9 ; Counter < 13 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 6 <= pos ) )
                {
                    MatrixData [ Counter ] [ 2 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 6 <= pos ) )
                {
                    MatrixData [ Counter ] [ 2 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 2 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 7:
            for ( Counter = 9 ; Counter < 13 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 7 <= pos ) )
                {
                    MatrixData [ Counter ] [ 7 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 7 <= pos ) )
                {
                    MatrixData [ Counter ] [ 7 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 7 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 8:
            for ( Counter = 9 ; Counter < 13 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 8 <= pos ) )
                {
                    MatrixData [ Counter ] [ 12 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 8 <= pos ) )
                {
                    MatrixData [ Counter ] [ 12 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 12 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 9:
            for ( Counter = 9 ; Counter < 13 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 9 <= pos ) )
                {
                    MatrixData [ Counter ] [ 17 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 9 <= pos ) )
                {
                    MatrixData [ Counter ] [ 17 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 17 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 10:
            for ( Counter = 9 ; Counter < 13 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 10 <= pos ) )
                {
                    MatrixData [ Counter ] [ 22 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 10 <= pos ) )
                {
                    MatrixData [ Counter ] [ 22 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 22 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 11:
            for ( Counter = 9 ; Counter < 13 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 11 <= pos ) )
                {
                    MatrixData [ Counter ] [ 27 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_RED_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_RED_TOP + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 11 <= pos ) )
                {
                    MatrixData [ Counter ] [ 27 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_GREEN_TOP + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 27 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_BLUE_TOP + MatrixRow [ Counter ];
                }
            }
        break;

        case 12:
            for ( Counter = 16 ; Counter < 20 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 12 <= pos ) )
                {
                    MatrixData [ Counter ] [ 2 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 12 <= pos ) )
                {
                    MatrixData [ Counter ] [ 2 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 2 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        case 13:
            for ( Counter = 16 ; Counter < 20 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 13 <= pos ) )
                {
                    MatrixData [ Counter ] [ 7 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 13 <= pos ) )
                {
                    MatrixData [ Counter ] [ 7 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 7 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        case 14:
            for ( Counter = 16 ; Counter < 20 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 14 <= pos ) )
                {
                    MatrixData [ Counter ] [ 12 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 14 <= pos ) )
                {
                    MatrixData [ Counter ] [ 12 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 12 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        case 15:
            for ( Counter = 16 ; Counter < 20 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 15 <= pos ) )
                {
                    MatrixData [ Counter ] [ 17 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 15 <= pos ) )
                {
                    MatrixData [ Counter ] [ 17 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 17 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        case 16:
            for ( Counter = 16 ; Counter < 20 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 16 <= pos ) )
                {
                    MatrixData [ Counter ] [ 22 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 16 <= pos ) )
                {
                    MatrixData [ Counter ] [ 22 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 22 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        case 17:
            for ( Counter = 16 ; Counter < 20 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 17 <= pos ) )
                {
                    MatrixData [ Counter ] [ 27 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 17 <= pos ) )
                {
                    MatrixData [ Counter ] [ 27 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 27 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        case 18:
            for ( Counter = 23 ; Counter < 27 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 18 <= pos ) )
                {
                    MatrixData [ Counter ] [ 2 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 18 <= pos ) )
                {
                    MatrixData [ Counter ] [ 2 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 2 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 3 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 4 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        case 19:
            for ( Counter = 23 ; Counter < 27 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 19 <= pos ) )
                {
                    MatrixData [ Counter ] [ 7 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 19 <= pos ) )
                {
                    MatrixData [ Counter ] [ 7 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 7 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 8 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 9 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        case 20:
            for ( Counter = 23 ; Counter < 27 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 20 <= pos ) )
                {
                    MatrixData [ Counter ] [ 12 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 20 <= pos ) )
                {
                    MatrixData [ Counter ] [ 12 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 12 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 13 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 14 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        case 21:
            for ( Counter = 23 ; Counter < 27 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 21 <= pos ) )
                {
                    MatrixData [ Counter ] [ 17 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 21 <= pos ) )
                {
                    MatrixData [ Counter ] [ 17 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 17 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 18 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 19 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        case 22:
            for ( Counter = 23 ; Counter < 27 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 22 <= pos ) )
                {
                    MatrixData [ Counter ] [ 22 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 22 <= pos ) )
                {
                    MatrixData [ Counter ] [ 22 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 22 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 23 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 24 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        case 23:
            for ( Counter = 23 ; Counter < 27 ; Counter++ )
            {
                if ( ( SENSOR_FAIL == state ) && ( 23 <= pos ) )
                {
                    MatrixData [ Counter ] [ 27 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_RED_BOTTOM + MatrixRow [ Counter ];
                }
                else if ( ( SENSOR_PASS == state ) && ( 23 <= pos ) )
                {
                    MatrixData [ Counter ] [ 27 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_GREEN_BOTTOM + MatrixRow [ Counter ];
                }
                else    // Invalid state
                {
                    MatrixData [ Counter ] [ 27 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 28 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                    MatrixData [ Counter ] [ 29 ] = LED_BLUE_BOTTOM + MatrixRow [ Counter ];
                }
            }
        break;

        default:
        break;
    }
}

void watchdog ( void )
{
    watchdog_update ( );
}

/*** end of file ***/
