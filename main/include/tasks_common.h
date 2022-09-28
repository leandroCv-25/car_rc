#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

/** Core 0
*Priority task
*/
// Wifi Reset Button task
#define WIFI_RESET_BUTTON_TASK_STACK_SIZE	1024
#define WIFI_RESET_BUTTON_TASK_PRIORITY		6
#define WIFI_RESET_BUTTON_TASK_CORE_ID		0

// WiFi application task
#define WIFI_APP_TASK_STACK_SIZE			4096
#define WIFI_APP_TASK_PRIORITY				5
#define WIFI_APP_TASK_CORE_ID				0

// HTTP Server task
#define HTTP_SERVER_TASK_STACK_SIZE			8192
#define HTTP_SERVER_TASK_PRIORITY			4
#define HTTP_SERVER_TASK_CORE_ID			0

// camera task
#define CAMERA_TASK_STACK_SIZE			    4096
#define CAMERA_TASK_PRIORITY			    3
#define CAMERA_TASK_CORE_ID			        0   

/** Core 1
*Priority task
*/
// CAr application task
#define CAR_APP_TASK_STACK_SIZE			 4096
#define CAR_APP_TASK_PRIORITY				1
#define CAR_APP_TASK_CORE_ID				1

#endif /* MAIN_TASKS_COMMON_H_ */