add_executable(uart_exercise uart_exercise.c)

# pull in common dependencies and additional uart hardware support
target_link_libraries(uart_exercise pico_stdlib hardware_uart)

# enable usb output, enable uart output
pico_enable_stdio_usb(uart_exercise 1)
pico_enable_stdio_uart(uart_exercise 0)

# create map/bin/hex file etc.
pico_add_extra_outputs(uart_exercise)

# add url via pico_set_program_url
example_auto_set_url(uart_exercise)
