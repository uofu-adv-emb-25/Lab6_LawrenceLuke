#include <stdio.h>
#include <pico/stdlib.h>
#include <stdint.h>
#include <unity.h>
#include "unity_config.h"
#include "busy.h"

void setUp(void) {}

void tearDown(void) {}

#define SUPERVISOR_PRIORITY ( tskIDLE_PRIORITY + 3UL )

#define HIGHER_PRIORITY     ( tskIDLE_PRIORITY + 2UL )
#define LOWER_PRIORITY      ( tskIDLE_PRIORITY + 1UL )
#define STACK_SIZE configMINIMAL_STACK_SIZE

TaskHandle_t task1;
TaskHandle_t task2;

void tester(TaskFunction_t func1, TaskFunction_t func2, UBaseType_t priority1, UBaseType_t priority2, uint64_t* time1, uint64_t* time2)
{
    xTaskCreate(func1, "Task1",
                STACK_SIZE, NULL, priority1, &task1);
    xTaskCreate(func2, "Task2",
                STACK_SIZE, NULL, priority2, &task2);
    
    vTaskDelay(1000);
    *time1 = ulTaskGetRunTimeCounter(task1);
    *time2 = ulTaskGetRunTimeCounter(task2);

    vTaskDelete(task1);
    vTaskDelete(task2);
}

void test_busy_busy_same()
{
    uint64_t time1 = 0, time2 = 0;

    tester(&busy_busy, &busy_busy, LOWER_PRIORITY, LOWER_PRIORITY, &time1, &time2);

    TEST_ASSERT_TRUE(time1 > time2 * .75 && time1 < time2 * 1.25);
}

void test_busy_yield_same()
{
    uint64_t time1 = 0, time2 = 0;

    tester(&busy_yield, &busy_yield, LOWER_PRIORITY, LOWER_PRIORITY, &time1, &time2);

    TEST_ASSERT_TRUE(time1 > time2 * .75 && time1 < time2 * 1.25);
}

void test_both_same()
{
    uint64_t time1 = 0, time2 = 0;

    tester(&busy_busy, &busy_yield, LOWER_PRIORITY, LOWER_PRIORITY, &time1, &time2);


    printf("Task 1 Runtime: %llu\n", time1);
    printf("Task 2 Runtime: %llu\n", time2);

    TEST_ASSERT_TRUE(time1 > time2 * 250 && time1 < time2 * 750);
}

void test_busy_busy_higher()
{
    uint64_t time1 = 0, time2 = 0;

    tester(&busy_busy, &busy_busy, HIGHER_PRIORITY, LOWER_PRIORITY, &time1, &time2);

    TEST_ASSERT_TRUE(time2 < time1 * .01);
}

void test_busy_busy_lower()
{
    uint64_t time1 = 0, time2 = 0;

    tester(&busy_busy, &busy_busy, LOWER_PRIORITY, HIGHER_PRIORITY, &time1, &time2);

    TEST_ASSERT_TRUE(time1 < time2 * .01);
}

void test_busy_yield_diff()
{
    uint64_t time1 = 0, time2 = 0;

    tester(&busy_yield, &busy_yield, LOWER_PRIORITY, HIGHER_PRIORITY, &time1, &time2);

    TEST_ASSERT_TRUE(time1 < time2 * .01);
}

void supervisor()
{
    while(1) {
        sleep_ms(5000); // Give time for TTY to attach.
        printf("Start tests\n");
        UNITY_BEGIN(); 
        RUN_TEST(test_busy_busy_same);
        RUN_TEST(test_busy_yield_same);
        RUN_TEST(test_both_same);
        RUN_TEST(test_busy_busy_higher);
        RUN_TEST(test_busy_busy_lower);
        RUN_TEST(test_busy_yield_diff);
        UNITY_END();
    }
}

int main (void)
{
    stdio_init_all();
    printf("Start main\n");
    xTaskCreate(supervisor, "supervisor",
                STACK_SIZE, NULL, SUPERVISOR_PRIORITY, NULL);
    vTaskStartScheduler();
}
