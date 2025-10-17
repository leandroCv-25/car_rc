set(PROJECT_NAME "controlCar")

idf_component_register(SRCS "app_communication.c" "app_nvs.c" "button_app.c" "headlight.c" "rgb_led.c" "car_app.c" "drive_motor.c" "servo.c" "main.c" "deep_sleep_app.c"
                    REQUIRES driver
                    INCLUDE_DIRS .)