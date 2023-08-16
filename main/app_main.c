/* Blink Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sdkconfig.h"
#include "input_iot.h"
#include "output_iot.h"
#include "freertos/event_groups.h"
#include "freertos/task.h"

#define BLINK_GPIO CONFIG_BLINK_GPIO
static EventGroupHandle_t xEventGroup;
#define BIT_PRESS_SHORT (1 << 0)
#define BIT_PRESS_NORMAL (1 << 1)
#define BIT_PRESS_LONG (1 << 2)
void input_event_callback(int pin, uint64_t tick)
{
    if (pin == GPIO_NUM_0)
    {
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        int press_ms = tick * portTICK_PERIOD_MS;
        if (press_ms < 1000)
        {
            xEventGroupSetBitsFromISR(
                xEventGroup,     /* The event group being updated. */
                BIT_PRESS_SHORT, /* The bits being set. */
                &xHigherPriorityTaskWoken);
        }
        else if (press_ms < 3000)
        {
            xEventGroupSetBitsFromISR(
                xEventGroup,      /* The event group being updated. */
                BIT_PRESS_NORMAL, /* The bits being set. */
                &xHigherPriorityTaskWoken);
        }
        else if (press_ms > 3000)
        {
            // xEventGroupSetBitsFromISR(
            //     xEventGroup,    /* The event group being updated. */
            //     BIT_PRESS_LONG, /* The bits being set. */
            //     &xHigherPriorityTaskWoken);
        }
    }
}
void vTaskCode(void *pvParameters)
{
    for (;;)
    {
        EventBits_t uxBits = xEventGroupWaitBits(
            xEventGroup,   /* The event group being tested. */
            BIT_PRESS_LONG|BIT_PRESS_NORMAL|BIT_PRESS_SHORT, /* The bits within the event group to wait for. */
            pdTRUE,        /* BIT_0 & BIT_4 should be cleared before returning. */
            pdFALSE,       /* Don't wait for both bits, either bit will do. */
            portMAX_DELAY); /* Wait a maximum of 100ms for either bit to be set. */

        if (uxBits & BIT_PRESS_SHORT)
        {
            printf("Press short \n");
        }
        else if (uxBits & BIT_PRESS_NORMAL)
        {
            printf("Press normal \n");
        }
        else if (uxBits & BIT_PRESS_LONG)
        {
            printf("Press long \n");
        }
    }
}
void button_timeout_callback(int pin)
{
    if (pin == BUTTON_0)
    {
        printf("TIMEOUT\n");
    }
}
void app_main(void)
{
    xEventGroup = xEventGroupCreate();
    output_io_create(BLINK_GPIO);
    input_io_create(GPIO_NUM_0, ANY_EDGE);
    input_set_callback(input_event_callback);
    input_set_timeout_callback(button_timeout_callback);
    xTaskCreate(
        vTaskCode,   /* Function that implements the task. */
        "vTaskCode", /* Text name for the task. */
        2048,        /* Stack size in words, not bytes. */
        NULL,        /* Parameter passed into the task. */
        4,           /* Priority at which the task is created. */
        NULL);       /* Used to pass out the created task's handle. */
}
