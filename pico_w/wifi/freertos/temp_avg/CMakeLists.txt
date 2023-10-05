add_executable(picow_freertos_temp_avg
    picow_freertos_temp_avg.c
    )
target_include_directories(picow_freertos_temp_avg PRIVATE
    ${CMAKE_CURRENT_LIST_DIR}
    ${CMAKE_CURRENT_LIST_DIR}/../..
)
target_link_libraries(picow_freertos_temp_avg
    hardware_adc
    pico_stdlib
    FreeRTOS-Kernel-Heap4 # FreeRTOS kernel and dynamic heap
    )

    pico_enable_stdio_usb(picow_freertos_temp_avg 1)
    pico_add_extra_outputs(picow_freertos_temp_avg)