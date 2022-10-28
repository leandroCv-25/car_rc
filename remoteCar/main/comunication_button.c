#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "esp_timer.h"

#include "tasks_common.h"
#include "comunication_button.h"
#include "comunication_app.h"

static const char TAG[] = "comunication_button";

gpio_num_t cButton;

/**
 * Comunication button task reacts to a button event by sending a message
 * to the comunication application to disconnect or connect from recever.
 * @param pvParam parameter which can be passed to the task.
 */
void comunication_button_task(void *pvParam)
{
	unsigned long lastDebounceTime = 0;
	int lastState = 0;
	while (true)
	{
		int reading = gpio_get_level(cButton);
		if (reading != lastState)
		{
			ESP_LOGI(TAG, "reset the debouncing timer");

			lastDebounceTime = esp_timer_get_time();
		}

		if ((esp_timer_get_time() - lastDebounceTime) > 4000000)
		{

			if (reading)
			{
				ESP_LOGI(TAG, "Action to the comunication");
				comunication_action_pairing();
				lastDebounceTime = esp_timer_get_time();
			}
		}
		lastState = reading;

		vTaskDelay(200 / portTICK_PERIOD_MS);
	}
}

void comunication_button_config(gpio_num_t comunicationButon)
{
	cButton = comunicationButon;
	// Configure the button and set the direction
	gpio_pad_select_gpio(cButton);
	gpio_set_direction(cButton, GPIO_MODE_INPUT);

	// Create the Wifi reset button task
	xTaskCreatePinnedToCore(&comunication_button_task, "comunication_button", COMUNICATION_BUTTON_TASK_STACK_SIZE, NULL, COMUNICATION_BUTTON_TASK_PRIORITY, NULL, COMUNICATION_BUTTON_TASK_CORE_ID);
}
