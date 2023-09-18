add_executable(hello_timer
        hello_timer.c
        )

# pull in common dependencies
target_link_libraries(hello_timer pico_stdlib)
pico_enable_stdio_usb(hello_timer 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(hello_timer)

# add url via pico_set_program_url
example_auto_set_url(hello_timer)
