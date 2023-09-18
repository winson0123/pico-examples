#include <stdio.h>
#include "pico/stdlib.h"

#define UART_ID uart0
#define BAUD_RATE 115200
#define UART_TX_PIN 16
#define UART_RX_PIN 17
#define BTN_PIN 15
#define DELAY 1000

// Definitions for character sending/processing
#define ASCII_UPPERCASE_A 'A'
#define ASCII_UPPERCASE_Z 'Z'
#define ASCII_DIGIT_1 '1'
#define ASCII_DIGIT_2 '2'

void init_gpio_uart()
{
    // Initialize the standard input and output (used for printf)
    stdio_usb_init();

    // Set up our UART with the required baud rate
    uart_init(UART_ID, BAUD_RATE);

    // Set the TX and RX pins for UART communication
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Configure our Pseudo Button with default HIGH state
    gpio_set_dir(BTN_PIN, GPIO_IN);
    gpio_set_pulls(BTN_PIN, true, false);
}

void process_uart_input()
{
    if (uart_is_readable(UART_ID))
    {
        char charRecv = uart_getc(UART_ID);

        // Process only if received a uppercase alphabetical characer
        if (charRecv >= ASCII_UPPERCASE_A && charRecv <= ASCII_UPPERCASE_Z)
        {
            // Convert uppercase to lowercase and print resultant character
            charRecv += 0x20;
            printf("%c\n", charRecv);
        }
        else if (charRecv == ASCII_DIGIT_1)
        {
            // Print '2' when '1' is received
            printf("%c\n", ASCII_DIGIT_2);
        }
    }
}

void send_uart_output(char *currentChar)
{
    if (gpio_get(BTN_PIN))
    {
        // GPIO is HIGH, send '1' through UART
        uart_putc(UART_ID, ASCII_DIGIT_1);
    }
    else
    {
        // GPIO is LOW, send alphabet character and increment
        uart_putc(UART_ID, (*currentChar)++);

        // Cycle alphabet character back to 'A', after 'Z' has been sent
        if (*currentChar > ASCII_UPPERCASE_Z)
        {
            *currentChar = ASCII_UPPERCASE_A;
        }
    }
}

int main()
{
    char alphabetChar = ASCII_UPPERCASE_A;

    // Initialize GPIO and UART
    init_gpio_uart();

    while (true)
    {
        // Read UART channel and process readable characeter
        process_uart_input();

        // Send uppercase alphabetical character to UART channel
        send_uart_output(&alphabetChar);

        // Sleep 1 second
        sleep_ms(DELAY);
    }
    return 0;
}