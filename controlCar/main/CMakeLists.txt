set(PROJECT_NAME "controlCar")

idf_component_register(SRCS "headlight.c" "rgb_led.c" "comunication_app.c" "car_app.c" "drive_motor.c" "servo.c" "main.c" "deep_sleep_app.c"
                    REQUIRES driver
                    INCLUDE_DIRS .)