add_executable(practical_test
practical_test.c
    )
target_link_libraries(practical_test
    pico_stdlib
    hardware_pwm
    )

pico_enable_stdio_usb(practical_test 1)
pico_add_extra_outputs(practical_test)
example_auto_set_url(practical_test)