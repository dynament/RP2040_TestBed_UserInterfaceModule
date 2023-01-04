
add_executable(src
        main.c
#        Adafruit_GFX.cpp
#        Adafruit_GrayOLED.cpp
#        Adafruit_Protomatter.cpp
#        Adafruit_SPITFT.cpp
#        core.c
#        glcdfont.c
#        RGBmatrixPanel.cpp
        )

# pico_enable_stdio_uart(src 1)

# pull in common dependencies
target_link_libraries(src
        pico_stdlib
        hardware_adc
        hardware_dma
        hardware_i2c
        hardware_pio
        hardware_spi
        hardware_watchdog
        hardware_uart
        hardware_irq
        hardware_sync
        hardware_timer
        hardware_irq
        hardware_pwm
        )

pico_enable_stdio_uart(src 0)
pico_enable_stdio_usb(src 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(src)

include_directories(
    ../inc
	)

## add url via pico_set_program_url
#//example_auto_set_url(adc)