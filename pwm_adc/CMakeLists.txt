add_executable(pwm_adc pwm_adc.c)

# pull in common dependencies, additional pwm hardware support and adc hardware support
target_link_libraries(pwm_adc pico_stdlib hardware_pwm hardware_adc)

# enable usb output
pico_enable_stdio_usb(pwm_adc 1)

# create map/bin/hex file etc.
pico_add_extra_outputs(pwm_adc)

# add url via pico_set_program_url
example_auto_set_url(pwm_adc)
