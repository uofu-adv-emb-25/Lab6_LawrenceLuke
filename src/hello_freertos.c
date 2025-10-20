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

void sub_task(void *params) {
    int delay = *((int*)params);
    if (delay)
        vTaskDelay(delay);
    
    if (xSemaphoreTake(sem, portMAX_DELAY))
        printf("Task%d took sem\n", delay);
}

void supervisor(__unused void *params) {
    sem = xSemaphoreCreateBinary(); 

    int delay0 = 0;
    xTaskCreate(sub_task, "Sub0",
                SUBORDINATE_STACK_SIZE, &delay0, SUBORDINATE_PRIORITY, NULL);

    int delay1 = 1;
    xTaskCreate(sub_task, "Sub1",
                SUBORDINATE_STACK_SIZE, &delay1, SUBORDINATE_PRIORITY + 1UL, NULL);
}

int main( void )
{
    stdio_init_all();
    hard_assert(cyw43_arch_init() == PICO_OK);
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, 1);
    printf("Started\n");
    const char *rtos_name;
    rtos_name = "FreeRTOS";
    TaskHandle_t task;
    xTaskCreate(supervisor, "Supervisor",
                SUPERVISOR_STACK_SIZE, NULL, SUPERVISOR_PRIORITY, &task);
    vTaskStartScheduler();
    return 0;
}
