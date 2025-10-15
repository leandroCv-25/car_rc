set(PROJECT_NAME "Remote Car")

idf_component_register(SRCS "app_communication.c" "deep_sleep_app.c" "button_app.c" "remote_app.c" "rgb_led.c" "main.c"
                    INCLUDE_DIRS "." include)