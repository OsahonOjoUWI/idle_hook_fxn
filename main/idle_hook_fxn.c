// Osahon Ojo - 816005001
// ECNG 3006 Lab #2

/* This code allows two tasks to share a GPIO pin using a mutex.
   This code also uses round-robin scheduling.
   This code also uses the IDLE "hook" function to put
   the processor to sleep for some time when the system
   is not going to be "in-use," i.e. when the idle task is running.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "freertos/semphr.h"

#include "driver/gpio.h"

#include "esp_log.h"
#include "esp_system.h"
#include "esp_timer.h"
#include "esp_sleep.h"

//for round robin
#define configUSE_PREEMPTION      1
#define configUSE_TIME_SLICING    1

//for idle hook
#define configUSE_IDLE_HOOK       1

#define GPIO_OUTPUT_PIN           2
#define GPIO_OUTPUT_PIN_SEL       (1ULL<<GPIO_OUTPUT_PIN)

gpio_config_t io_conf;
SemaphoreHandle_t xMutex = NULL;

//the task needs an infinite loop
//otherwise, it (the task) will terminate
//in other words, the funciton's lifetime is
//throughout the program. Hence, static.
static void turn_pin_on()
{
    //the counter variable's lifetime if throughout the program
    //hence, static.
    static int counter = 0;

    printf("At turn_pin_on: entry\n");

    for (;;)
    {
        printf("At turn_pin_on: infinite for loop iteration\n");

        // turn GPIO pin on - check mutex first
        if (xSemaphoreTake(xMutex, (TickType_t) 10))
        {
            gpio_set_level(GPIO_OUTPUT_PIN, 1);
            xSemaphoreGive(xMutex);
        }

        // actively wait for 0.5s
        long nTicks = (long) xTaskGetTickCount();
        long nTicksPlus500 = nTicks + 500;
        printf("At turn_pin_on: tick count before 0.5s active wait: 0x%x\n", xTaskGetTickCount());
        do
        {
            counter++;
            if (counter % 400000 == 0)
                printf("At turn_pin_on: busy waiting\n");
        }
        while ((long)xTaskGetTickCount() < nTicksPlus500);

        counter = 0;
        printf("At turn_pin_on: tick count after 0.5s active wait: 0x%x\n", xTaskGetTickCount());

        // task-delay for 1s
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    printf("At turn_pin_on: exit\n");
}

//the task needs an infinite loop
//otherwise, it (the task) will terminate
//in other words, the funciton's lifetime is
//throughout the program. Hence, static.
static void turn_pin_off()
{
    //the counter variable's lifetime if throughout the program
    //hence, static.
    static int counter = 0;

    printf("At turn_pin_off: entry\n");

    for (;;)
    {
        printf("At turn_pin_off: infinite for loop iteration\n");

        // turn GPIO pin off - check mutex first
        if (xSemaphoreTake(xMutex, (TickType_t) 10))
        {
            gpio_set_level(GPIO_OUTPUT_PIN, 0);
            xSemaphoreGive(xMutex);
        }

        // actively wait for 0.5s
        long nTicks = (long) xTaskGetTickCount();
        long nTicksPlus500 = nTicks + 500;
        printf("At turn_pin_off: tick count before 0.5s active wait: 0x%x\n", xTaskGetTickCount());
        do
        {
            counter++;
            if (counter % 400000 == 0)
                printf("At turn_pin_off: busy waiting\n");
        }
        while ((long)xTaskGetTickCount() < nTicksPlus500);

        counter = 0;
        printf("At turn_pin_off: tick count after 0.5s active wait: 0x%x\n", xTaskGetTickCount());

        // task-delay for 1s
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    printf("At turn_pin_off: exit\n");
}

//the task needs an infinite loop
//otherwise, it (the task) will terminate
//in other words, the funciton's lifetime is
//throughout the program. Hence, static.
static void print_status()
{
    printf("At print_status: entry\n");

    for (;;)
    {
        printf("At print_status: infinite for loop iteration\n");

        // print status message
        int level = gpio_get_level(GPIO_OUTPUT_PIN);
        if (level == 0)
            printf("GPIO pin %d is LOW\n", GPIO_OUTPUT_PIN);
        else
            printf("GPIO pin %d is HIGH\n", GPIO_OUTPUT_PIN);

        // task-delay for 1s
        vTaskDelay(1000 / portTICK_RATE_MS);
    }

    printf("At print_status: exit\n");
}

void vApplicationIdleHook()
{
    //put ESP to sleep for 500us
    printf("At vApplicationIdleHook: entering sleep for 500us\n");
    esp_sleep_enable_timer_wakeup(500);
    esp_light_sleep_start();
}

void app_main(void)
{
    printf("At app_main: entry\n");

    printf("At app_main: GPIO configuration\n");

    //let io_conf be global variable so that the pin configuration
    //is not lost when app_main terminates
    //gpio_config_t io_conf;

    // disable interrupt
    io_conf.intr_type = GPIO_INTR_DISABLE;
    // set as output mode
    io_conf.mode = GPIO_MODE_OUTPUT;
    // bit mask of the pins that you want to set,e.g.GPIO2
    io_conf.pin_bit_mask = GPIO_OUTPUT_PIN_SEL;
    // disable pull-down mode
    io_conf.pull_down_en = 0;
    // disable pull-up mode
    io_conf.pull_up_en = 0;
    // configure GPIO with the given settings
    gpio_config(&io_conf);

    // create mutex
    printf("At app_main: xSemaphoreCreateMutex()\n");
    xMutex = xSemaphoreCreateMutex();

    // set GPIO pin high initially
    gpio_set_level(GPIO_OUTPUT_PIN, 1);

    // start tasks
    // xTaskCreate(task_function, task_name_for_logs, stack_depth_in_words, void* task_parameters, task_priority, task_name_as_return_value_of_xTaskCreate)
    printf("At app_main: xTaskCreate function calls\n");
    xTaskCreate(turn_pin_on, "turn_pin_on", 2048, NULL, 10, NULL);
    xTaskCreate(turn_pin_off, "turn_pin_off", 2048, NULL, 9, NULL);
    xTaskCreate(print_status, "print_status", 2048, NULL, 8, NULL);

    /*while(1)
    {
        printf("At app_main: infinite while loop\n");
    }*/

    printf("At app_main: exit\n");
}
