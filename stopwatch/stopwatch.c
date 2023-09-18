#include <stdio.h>
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#define BTN_PIN 15
#define DEBOUNCE_DELAY_MS 500   // Debounce delay: 500 milliseconds
#define PRINT_INTERVAL_MS -1000 // 1 second interval for stopwatch print

// Global variables to keep track of button state and time
volatile bool lastButtonState = false;
volatile uint64_t lastDebouncedTime = 0;
volatile uint64_t stopwatchStartTime = 0;

void init_gpio()
{
    // Initialize the standard input and output (used for printf)
    stdio_usb_init();

    // Configure our Pseudo Button to default LOW state
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_set_pulls(BTN_PIN, false, true);
}

bool print_stopwatch(struct repeating_timer *t)
{
    // Calculate the elapsed time in seconds by subtracting  the start time
    // from the current time and displays it with two decimal places
    float seconds = (float)(time_us_64() - stopwatchStartTime) / 1000000;
    printf("Stopwatch: %04.2f\n", seconds);
    return true;
}

bool debounce_button(uint gpio)
{
    // Time-based debounce algorithm which ensures that the button state
    // change is properly debounced before registering it
    uint64_t currentTime = time_us_64();
    bool currentButtonState = gpio_get(gpio);

    // Check if the current time has passed the debounce delay
    // and if the button state has changed
    if (currentTime - lastDebouncedTime > DEBOUNCE_DELAY_MS * 1000 && currentButtonState != lastButtonState)
    {
        // update the lastDebouncedTime- the time when the button state changed
        lastDebouncedTime = currentTime;
        printf("Pseudo-button toggled\n");

        // return the new button state
        return currentButtonState;
    }
    // return the last button state, since no change to state
    return lastButtonState;
}

// bool debounce_button(uint gpio)
// {
//     // Debouncing algorithm based on checking the button's state multiple
//     // times consecutively before considering it as a valid state change
//     // *not the most efficient, while loop introduces unnecessary delay
//     uint64_t currentTime = time_us_64();
//     bool currentButtonState = gpio_get(gpio);
//     int consecutiveCount = 1000;
//     if (currentButtonState != lastButtonState){
//         while (consecutiveCount != 0){
//             if (gpio_get(gpio) != currentButtonState)
//                 return lastButtonState;
//             consecutiveCount--;
//         }
//         lastDebouncedTime = currentTime;
//         printf("Pseudo-button triggered\n");
//         return currentButtonState;
//     }
//     return lastButtonState;
// }

void gpio_callback(uint gpio, uint32_t events)
{
    // obtain button state based on the debouncing algorithm
    lastButtonState = debounce_button(gpio);
}

bool start_stopwatch(repeating_timer_t *stopwatch)
{
    // capture the debounced time as the start time of the stopwatch
    // when the 3.3V LIVE connector is connected to GPIO PIN 15
    stopwatchStartTime = lastDebouncedTime;

    // create a repeating timer to print the stopwatch time every 1 second
    add_repeating_timer_ms(PRINT_INTERVAL_MS, print_stopwatch, NULL, stopwatch);

    // indicate that the stopwatch has started
    printf("Stopwatch started\n");
    printf("Stopwatch: %04.2f\n", 0);

    // enable stopwatchRunning
    return true;
}

bool stop_stopwatch(repeating_timer_t *stopwatch)
{
    // capture the debounced time as the stop time of the stopwatch
    // when the 3.3V LIVE connector is disconnected from GPIO PIN 15
    uint64_t stopwatchStopTime = lastDebouncedTime;

    // Stop the repeating timer associated with the stopwatch
    cancel_repeating_timer(stopwatch);

    // calculate the total elapsed time in seconds
    float totalElapsedTime = (float)(stopwatchStopTime - stopwatchStartTime) / 1000000;

    // display the final stopwatch time with two decimal places for precision
    printf("Stopwatch: %04.2f\n", totalElapsedTime);
    printf("Stopwatch stopped\n");

    // disable stopwatchRunning
    return false;
}

int main()
{
    init_gpio();
    // Enable GPIO interrupt on rising and falling edges
    gpio_set_irq_enabled_with_callback(BTN_PIN, GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL, true, &gpio_callback);
    struct repeating_timer stopwatch;
    volatile bool stopwatchRunning = false;

    while (true)
    {
        if (lastButtonState && !stopwatchRunning)
        {
            stopwatchRunning = start_stopwatch(&stopwatch);
        }
        else if (!lastButtonState && stopwatchRunning)
        {
            stopwatchRunning = stop_stopwatch(&stopwatch);
        }
        else
        {
            // do nothing
            tight_loop_contents();
        }
    }

    return 0;
}
