/*
__author__ = "@winson0123"
 */

#include <stdio.h>
#include "pico/stdlib.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 16
#define UART_RX_PIN 17
#define BTN_PIN 15
#define DELAY 1000

void init_gpio_uart() {
    // Initialize the standard input and output (used for printf)
    stdio_usb_init();

    // Set up our UART with the required baud rate
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins for UART communication
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Configure our Pseudo Button
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_set_pulls(BTN_PIN, true, false);
}

int main() {
    char charData = 'A';

    // Initialize GPIO and UART
    init_gpio_uart();

    while (true) {
        if(uart_is_readable(UART_ID)){
            char charRecv = uart_getc(UART_ID);
            if (charRecv >= 'A' && charRecv <= 'Z')
                // arithmetic transformation to lowercase
                printf("%c\n", charRecv + 0x20);
            else if (charRecv == '1'){
                // print out the number '2'
                printf("%c\n", '2');
            }
        }

        if (gpio_get(BTN_PIN)) {
            // GPIO is HIGH, send '1' through UART
            uart_putc(UART_ID, '1');
        } else {
            // GPIO is LOW, send alphabet and cycle through 'A' to 'Z'
            uart_putc(UART_ID, charData++);
            if (charData > 'Z') {
                charData = 'A';
            }
        }
        // Sleep 1 second
        sleep_ms(DELAY);
    }
    return 0;
}