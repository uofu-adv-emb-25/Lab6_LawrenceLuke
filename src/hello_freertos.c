/**
 * Copyright (c) 2022 Raspberry Pi (Trading) Ltd.
 *
 * SPDX-License-Identifier: BSD-3-Clause
 */

#include <stdio.h>

#include "FreeRTOS.h"
#include "task.h"

#include "pico/stdlib.h"
#include "pico/multicore.h"
#include "pico/cyw43_arch.h"

#include <semphr.h>

#define SUPERVISOR_PRIORITY      ( tskIDLE_PRIORITY + 3UL )
#define SUBORDINATE_PRIORITY     ( tskIDLE_PRIORITY + 1UL )
#define SUPERVISOR_STACK_SIZE configMINIMAL_STACK_SIZE
#define SUBORDINATE_STACK_SIZE configMINIMAL_STACK_SIZE

SemaphoreHandle_t sem;
int delay0 = 0;
int delay1 = 1;

void sub_task(void *params) {
    int delay = *((int*)params);
    if (delay)
        vTaskDelay(delay);
    
    if (xSemaphoreTake(sem, 1000))
        printf("Task%d took sem\n", delay);
    else
        printf("Task%d failed to take sem\n", delay);
    
    while (1)
        vTaskDelay(100);
}

void supervisor(__unused void *params) {
    sem = xSemaphoreCreateMutex(); 

    xTaskCreate(sub_task, "Sub0",
                SUBORDINATE_STACK_SIZE, &delay0, SUBORDINATE_PRIORITY, NULL);

    xTaskCreate(sub_task, "Sub1",
                SUBORDINATE_STACK_SIZE, &delay1, SUBORDINATE_PRIORITY + 1UL, NULL);
    while (1)
        vTaskDelay(100);
}

int main( void )
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    sleep_ms(5000);
    
    printf("Started\n");
    const char *rtos_name;
    rtos_name = "FreeRTOS";
    TaskHandle_t task;
    xTaskCreate(supervisor, "Supervisor",
                SUPERVISOR_STACK_SIZE, NULL, SUPERVISOR_PRIORITY, &task);
    vTaskStartScheduler();
    return 0;
}
