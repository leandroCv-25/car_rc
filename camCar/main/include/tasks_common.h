#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

/** Core 0
*Priority task
*/

// WiFi application task
#define WIFI_APP_TASK_STACK_SIZE			4096
#define WIFI_APP_TASK_PRIORITY				2
#define WIFI_APP_TASK_CORE_ID				0

// HTTP Server task
#define HTTP_SERVER_TASK_STACK_SIZE			8192
#define HTTP_SERVER_TASK_PRIORITY			1
#define HTTP_SERVER_TASK_CORE_ID			0


/** Core 1
*Priority task
*/
// camera task
#define CAMERA_TASK_STACK_SIZE			   8192
#define CAMERA_TASK_PRIORITY			    0
#define CAMERA_TASK_CORE_ID			        1 

#endif /* MAIN_TASKS_COMMON_H_ */