#include <stdio.h>
#include <stdarg.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"
#include "hardware/adc.h"

#define PWM_PIN 2
#define ADC_PIN 26
#define PWM_FREQUENCY 20
#define PWM_WRAP_VALUE 62500
#define PRINT_INTERVAL_MS -25 // 25ms interval for adc print

void init_gpio()
{
    // initialize the standard input and output (used for printf)
    stdio_usb_init();

    // configure GPIO for PWM to be allocated to the PWM
    gpio_set_function(PWM_PIN, GPIO_FUNC_PWM);

    // find out which PWM slice is connected to GPIO PWN_PIN (it's slice 2)
    uint slice_num = pwm_gpio_to_slice_num(PWM_PIN);

    // set PWM clock divider and wrap value for the 20 Hz frequency
    pwm_set_clkdiv(slice_num, PWM_FREQUENCY);
    pwm_set_wrap(slice_num, PWM_WRAP_VALUE);

    // set channel A duty cycle to 50% (half of wrap value)
    pwm_set_chan_level(slice_num, PWM_CHAN_A, PWM_WRAP_VALUE / 2);

    // enable the PWM output
    pwm_set_enabled(slice_num, true);

    // configure GPIO for ADC
    adc_init();
    adc_set_temp_sensor_enabled(true);

    // set GPIO ADC_PIN function to SIO
    gpio_set_function(ADC_PIN, GPIO_FUNC_SIO);

    // disable pulls and input enable for ADC_PIN
    gpio_disable_pulls(ADC_PIN);
    gpio_set_input_enabled(ADC_PIN, false);
}

void printf_timestamp(const char *format, ...)
{
    // Wrapper function to print the timestamp along with the intended
    // print statement:
    // HH:MM:SS:NNN -> {string_to_print}
    va_list args;
    va_start(args, format);

    uint64_t timestamp_us = time_us_64();
    uint32_t milliseconds = (timestamp_us / 1000) % 1000;
    uint32_t seconds = (timestamp_us / 1000000) % 60;
    uint32_t minutes = (timestamp_us / 60000000) % 60;
    uint32_t hours = (timestamp_us / 3600000000) % 24;

    // print leading timestamp
    printf("%02u:%02u:%02u:%03u -> ", hours, minutes, seconds, milliseconds);

    // call the original printf with variable arguments
    vprintf(format, args);

    va_end(args);
}

bool print_adc_value(struct repeating_timer *t)
{
    // read the value from ADC
    uint16_t adc_value = adc_read();

    //  print ADC value with timestamp
    printf_timestamp("ADC Value: %u\n", adc_read());
    return true;
}

int main()
{
    init_gpio();

    // create a repeating timer for ADC value printing every 25ms
    struct repeating_timer adc_timer;
    add_repeating_timer_ms(PRINT_INTERVAL_MS, print_adc_value, NULL, &adc_timer);

    while (true)
    {
        // do nothing
        tight_loop_contents();
    }
}
