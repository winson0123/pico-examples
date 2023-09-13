add_executable(picow_blink_button
        picow_blink_button.c
        )

# pull in common dependencies
target_link_libraries(picow_blink_button pico_stdlib pico_cyw43_arch_none)

# create map/bin/hex file etc.
pico_add_extra_outputs(picow_blink_button)

# add url via pico_set_program_url
example_auto_set_url(picow_blink_button)
