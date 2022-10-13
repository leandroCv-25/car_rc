#ifndef COMUNICATION_APP_H_
#define COMUNICATION_APP_H_

typedef enum {
    ESPNOW_CTRL_INIT,
    ESPNOW_CTRL_BOUND,
    ESPNOW_CTRL_MAX
} espnow_ctrl_status_t;

/**
 * @brief  the Comunication task - uses esp-now
 * 
 */
void comunication_app_start(void);

#endif /* COMUNICATION_APP_H_ */