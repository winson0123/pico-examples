#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

#define START_LED 2
#define END_LED 13
#define BREATHE_LED 16
#define PWM_FREQUENCY 20
#define PWM_WRAP_VALUE 62500
#define START_PAUSE_BTN 20
#define LAP_BTN 21
#define RESET_BTN 22
#define MAX_COUNT 500

#define PRINT_INTERVAL_MS -20 // 0.02 second interval for stopwatch print

int stopwatchTime = 0;
struct repeating_timer stopwatch;
struct repeating_timer breathe_led;
bool stopwatchRunning = false;
bool stop = false;
int duty = 0;

void show_led_binary(int current_time){
    int binaryNum[12] = {0};
    int i = 12;
    while (current_time > 0){
        binaryNum[i--] = current_time % 2;
        current_time /= 2;
    }

    i = 0;
    for (int j = START_LED; j <= END_LED; j++){
        if (binaryNum[i++])
            gpio_set_pulls(j, true, false);
        else
            gpio_set_pulls(j, false, true);
    }
}

bool print_stopwatch(struct repeating_timer *t){
    if (stopwatchTime > MAX_COUNT){
        cancel_repeating_timer(&stopwatch);
        stopwatchRunning = false;
        stop = true;
        return true;
    }

    printf("Time: %d ms\n", stopwatchTime);
    show_led_binary(stopwatchTime);
    stopwatchTime += 1;
    return true;
}

void start_stopwatch(){
    add_repeating_timer_ms(PRINT_INTERVAL_MS, print_stopwatch, NULL, &stopwatch);
    duty = 0;
    cancel_repeating_timer(&breathe_led);
}

bool breathing_led(struct repeating_timer *t){
    pwm_set_chan_level(pwm_gpio_to_slice_num(BREATHE_LED), pwm_gpio_to_channel(BREATHE_LED), (PWM_WRAP_VALUE/100) * duty);
    duty++;
    if (duty > 100){
        duty = 0;
    }
    return true;
}

void pause_stopwatch(){
    cancel_repeating_timer(&stopwatch);
    add_repeating_timer_ms(1, breathing_led, NULL, &breathe_led);
    printf("Pause: %d ms\n", stopwatchTime - 1);
}

void lap_stopwatch(){
    printf("Lap: %d ms\n", stopwatchTime - 1);
}

void reset_led(){
    for (int i = START_LED; i <= END_LED; i++){
        gpio_set_pulls(i, false, true);
    }
    pwm_set_chan_level(pwm_gpio_to_slice_num(BREATHE_LED), pwm_gpio_to_channel(BREATHE_LED), 0);
}

void reset_stopwatch(){
    stopwatchTime = 0;
    reset_led();
    cancel_repeating_timer(&stopwatch);
    cancel_repeating_timer(&breathe_led);
    duty = 0;
    printf("Time: %d ms\n", stopwatchTime);
}

void toggle_stopwatch(uint gpio){
    printf("button toggled %d\n", gpio);
    if (gpio == START_PAUSE_BTN && !stopwatchRunning && !stop){
        start_stopwatch();
        stopwatchRunning = true;
    } else if (gpio == START_PAUSE_BTN && stopwatchRunning){
        pause_stopwatch();
        stopwatchRunning = false;
    }
    if (gpio == LAP_BTN && stopwatchRunning) {
        lap_stopwatch();
    }
    if (gpio == RESET_BTN && (stopwatchTime > MAX_COUNT || !stopwatchRunning)){
        reset_stopwatch();
        stop = false;
    }
}

void gpio_callback(uint gpio, uint32_t events)
{
    toggle_stopwatch(gpio);
}

void init_gpio(){
    stdio_usb_init();
    // Configure our Buttons to default HIGH state
    gpio_set_dir(START_PAUSE_BTN, GPIO_IN);
    gpio_set_pulls(START_PAUSE_BTN, false, true);
    gpio_set_dir(LAP_BTN, GPIO_IN);
    gpio_set_pulls(LAP_BTN, false, true);
    gpio_set_dir(RESET_BTN, GPIO_IN);
    gpio_set_pulls(RESET_BTN, false, true);

    // Configure our LEDs to be turned off
    for (int i = START_LED; i <= END_LED; i++){
        gpio_set_dir(i, GPIO_OUT);
        gpio_set_pulls(i, false, true);
    }

    // configure GPIO for PWM to be allocated to the PWM
    gpio_set_function(BREATHE_LED, GPIO_FUNC_PWM);
    uint slice_num = pwm_gpio_to_slice_num(BREATHE_LED);

    // set PWM clock divider and wrap value for the 20 Hz frequency
    pwm_set_clkdiv(slice_num, PWM_FREQUENCY);
    pwm_set_wrap(slice_num, PWM_WRAP_VALUE);
    pwm_set_chan_level(slice_num, pwm_gpio_to_channel(BREATHE_LED), 0);
    pwm_set_enabled(slice_num, true);
}

int main() {
    init_gpio();
    gpio_set_irq_enabled_with_callback(START_PAUSE_BTN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(LAP_BTN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    gpio_set_irq_enabled_with_callback(RESET_BTN, GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    
    while (1){
        tight_loop_contents();
    }
    return 0;
}