#include <stdio.h>
#include <string.h>
#include "FreeRTOS.h"
#include "hardware/adc.h"
#include "hardware/gpio.h"
#include "message_buffer.h"
#include "pico/stdlib.h"
#include "task.h"

#ifndef RUN_FREERTOS_ON_CORE
#define RUN_FREERTOS_ON_CORE 0
#endif

#define DELAY_MS 1000
#define MESSAGE_BUFFER_SIZE 60
#define MOVING_AVERAGE_BUFFER_SIZE 10

static MessageBufferHandle_t movingAverageMessageBuffer;
static MessageBufferHandle_t simpleAverageMessageBuffer;
static MessageBufferHandle_t printMessageBuffer;

float read_onboard_temperature()
{
    // 12-bit conversion, assume max value == ADC_VREF == 3.3 V
    const float conversionFactor = 3.3f / (1 << 12);

    // Read ADC value and convert it to temperature in Celsius
    float adc = (float)adc_read() * conversionFactor;
    float tempC = 27.0f - (adc - 0.706f) / 0.001721f;

    return tempC;
}

void temp_task(__unused void *params)
{
    // Task that reads the temperature data from the RP2040's built-in 
    // temperature sensor and sends it to two tasks every 1 second
    float temperature = 0.0;
    adc_init();
    adc_set_temp_sensor_enabled(true);
    adc_select_input(4);

    while (true)
    {
        // Use pdMS_TO_TICKS to specify delay in milliseconds
        vTaskDelay(pdMS_TO_TICKS(DELAY_MS));

        // Read current temperature
        temperature = read_onboard_temperature();

        // Send temperature data to other tasks
        xMessageBufferSend(movingAverageMessageBuffer, &temperature, sizeof(temperature), 0);
        xMessageBufferSend(simpleAverageMessageBuffer, &temperature, sizeof(temperature), 0);
    }
}

void moving_avg_task(__unused void *params)
{
    // Task will perform a moving average on a buffer of ten data points
    float data[MOVING_AVERAGE_BUFFER_SIZE] = {0};
    float sum = 0;
    int index = 0;
    int count = 0;

    float fReceivedData = 0;
    size_t xReceivedBytes = 0;
    char message[MESSAGE_BUFFER_SIZE];

    while (true)
    {
        xReceivedBytes = xMessageBufferReceive(
            movingAverageMessageBuffer,
            (void *)&fReceivedData,
            sizeof(fReceivedData),
            portMAX_DELAY);

        // Update moving average buffer and calculate new average
        sum -= data[index];
        data[index] = fReceivedData;
        sum += data[index];
        index = (index + 1) % MOVING_AVERAGE_BUFFER_SIZE;
        if (count < MOVING_AVERAGE_BUFFER_SIZE)
            // Increment count till it reaches MOVING_AVERAGE_BUFFER_SIZE
            count++;

        // Format and send the message to print_task
        memset(message, 0, sizeof(message));
        sprintf(message, "Moving Average Temperature = %.2f C\n", sum / count);
        xMessageBufferSend(printMessageBuffer, (void *)message, strlen(message) + 1, 0);
    }
}

void simple_avg_task(__unused void *params)
{
    // Note: Buffer overflow is unlikely as long as the sum of temperature 
    // values remains below 3.4 x 10^38, which is well within the limits for
    // this simple averaging task.
    // Task will perform a simple averaging
    float sum = 0;
    int count = 0;

    float fReceivedData = 0;
    size_t xReceivedBytes = 0;
    char message[MESSAGE_BUFFER_SIZE];

    while (true)
    {
        xReceivedBytes = xMessageBufferReceive(
            simpleAverageMessageBuffer,
            (void *)&fReceivedData,
            sizeof(fReceivedData),
            portMAX_DELAY);

        // Update sum and count for simple average
        sum += fReceivedData;
        count++;

        // Format and send the message to print_task
        memset(message, 0, sizeof(message));
        sprintf(message, "Simple Average Temperature = %.2f C\n", sum / count);
        xMessageBufferSend(printMessageBuffer, (void *)message, strlen(message) + 1, 0);
    }
}

void print_task(__unused void *params)
{
    // Task exclusively for executing all the printf statements
    size_t xReceivedBytes = 0;
    char message[MESSAGE_BUFFER_SIZE];

    while (true)
    {
        memset(message, 0, sizeof(message));
        xReceivedBytes = xMessageBufferReceive(
            printMessageBuffer,
            (void *)message,
            MESSAGE_BUFFER_SIZE,
            portMAX_DELAY);

        // Print the message received in the buffer from other tasks
        printf("%s", message);
    }
}

void vLaunch(void)
{
    TaskHandle_t tempTask;
    TaskHandle_t movingAverageTask;
    TaskHandle_t simpleAverageTask;
    TaskHandle_t printTask;

    // Create message buffers
    movingAverageMessageBuffer = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);
    simpleAverageMessageBuffer = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);
    printMessageBuffer = xMessageBufferCreate(MESSAGE_BUFFER_SIZE);

    // Create tasks with appropriate priorities
    xTaskCreate(temp_task, "TempTask", configMINIMAL_STACK_SIZE, NULL, 1, &tempTask);
    xTaskCreate(moving_avg_task, "MovingAvgTask", configMINIMAL_STACK_SIZE, NULL, 2, &movingAverageTask);
    xTaskCreate(simple_avg_task, "SimpleAvgTask", configMINIMAL_STACK_SIZE, NULL, 2, &simpleAverageTask);
    xTaskCreate(print_task, "PrintTask", configMINIMAL_STACK_SIZE, NULL, 3, &printTask);

#if NO_SYS && configUSE_CORE_AFFINITY && configNUM_CORES > 1
    // we must bind the main task to one core (well at least while the init is called)
    // (note we only do this in NO_SYS mode, because cyw43_arch_freertos
    // takes care of it otherwise)
    vTaskCoreAffinitySet(task, 1);
#endif

    /* Start the tasks and timer running. */
    vTaskStartScheduler();
}

int main(void)
{
    stdio_init_all();

    /* Configure the hardware ready to run the demo. */
    const char *rtos_name;
#if (portSUPPORT_SMP == 1)
    rtos_name = "FreeRTOS SMP";
#else
    rtos_name = "FreeRTOS";
#endif

#if (portSUPPORT_SMP == 1) && (configNUM_CORES == 2)
    printf("Starting %s on both cores:\n", rtos_name);
    vLaunch();
#elif (RUN_FREERTOS_ON_CORE == 1)
    printf("Starting %s on core 1:\n", rtos_name);
    multicore_launch_core1(vLaunch);
    while (true)
        ;
#else
    printf("Starting %s on core 0:\n", rtos_name);
    vLaunch();
#endif
    return 0;
}