add_executable(pid
    pid.c
    )
target_link_libraries(pid
    pico_stdlib 
    )

pico_enable_stdio_usb(pid 1)
pico_add_extra_outputs(pid)
example_auto_set_url(pid)