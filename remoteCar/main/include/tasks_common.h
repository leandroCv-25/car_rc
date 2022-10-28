#ifndef MAIN_TASKS_COMMON_H_
#define MAIN_TASKS_COMMON_H_

/** Core 0
*Priority task
*/
// Wifi Reset Button task
#define COMUNICATION_BUTTON_TASK_STACK_SIZE	    4096
#define COMUNICATION_BUTTON_TASK_PRIORITY		1
#define COMUNICATION_BUTTON_TASK_CORE_ID		0

// WiFi application task
#define COMUNICATION_APP_TASK_STACK_SIZE	4096
#define COMUNICATION_APP_TASK_PRIORITY		0
#define COMUNICATION_APP_TASK_CORE_ID		0

/** Core 1
*Priority task
*/
// Remote application task
#define REMOTE_APP_TASK_STACK_SIZE			    4096
#define REMOTE_APP_TASK_PRIORITY				0
#define REMOTE_APP_TASK_CORE_ID				    1

#endif /* MAIN_TASKS_COMMON_H_ */