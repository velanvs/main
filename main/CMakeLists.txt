idf_component_register(SRCS "servo.c" "app.c" "gpio.c" "mqtt.c" "wifi_station.c" "blink_example_main.c" "soil.c"
                       INCLUDE_DIRS ".")
target_compile_options(${COMPONENT_LIB} PRIVATE -Wno-error)
