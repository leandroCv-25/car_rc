#ifndef COMUNICATION_APP_H_
#define COMUNICATION_APP_H_

typedef enum {
    ESPNOW_CTRL_INIT,
    ESPNOW_CTRL_BOUND,
    ESPNOW_CTRL_MAX
} espnow_ctrl_status_t;

typedef enum {
    ANGLE_CONTROL_DATA = 0,
    HEADLIGHT_CONTROL_DATA = 1,
    ACELERATION_CONTROL_DATA = 2,
} kind_control_data_t;

/**
 * @brief  the Comunication task - uses esp-now
 * 
 */
void comunication_app_start(void);

/**
 * @brief  the Comunication action pairing - uses esp-now
 * 
 */
void comunication_action_pairing(void);

/**
 * @brief  the Comunication send mensage - uses esp-now
 * 
 * @param kindControlData Id the kind of control mensage
 * 
 * @param data send data
 * 
 */
void comunication_send(kind_control_data_t kindControlData, uint32_t data);

#endif /* COMUNICATION_APP_H_ */