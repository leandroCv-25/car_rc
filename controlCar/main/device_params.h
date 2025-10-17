#ifndef DEVICE_PARAMS_H_
#define DEVICE_PARAMS_H_

/** Core 0
 *Priority task
 */
// WiFi application task
#define APP_COMUNICATION_STACK 12000
#define APP_COMUNICATION_PRIORITY 1
#define APP_COMUNICATION_CORE_ID 0

/** Core 1
 *Priority task
 */
// Remote application task
#define CAR_APP_TASK_STACK_SIZE 8184
#define CAR_APP_TASK_PRIORITY 1
#define CAR_APP_TASK_CORE_ID 1

#define DEEP_SLEEP_APP_TASK_STACK_SIZE 1024
#define DEEP_SLEEP_APP_TASK_PRIORITY 0
#define DEEP_SLEEP_APP_TASK_CORE_ID 1

#endif /* MAIN_TASKS_COMMON_H_ */