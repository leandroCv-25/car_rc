#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

/** Core 0
*Priority task
*/
// Wifi Reset Button task
#define WIFI_RESET_BUTTON_TASK_STACK_SIZE	1024
#define WIFI_RESET_BUTTON_TASK_PRIORITY		0
#define WIFI_RESET_BUTTON_TASK_CORE_ID		0


/** Core 1
*Priority task
*/
// CAr application task
#define CAR_APP_TASK_STACK_SIZE			    4096
#define CAR_APP_TASK_PRIORITY				0
#define CAR_APP_TASK_CORE_ID				1

#define DEEP_SLEEP_APP_TASK_STACK_SIZE		1024
#define DEEP_SLEEP_APP_TASK_PRIORITY		6
#define DEEP_SLEEP_APP_TASK_CORE_ID			1

#endif /* MAIN_TASKS_COMMON_H_ */