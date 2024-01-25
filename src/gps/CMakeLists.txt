idf_component_register(
    SRCS "gps_example_main.c" "gps.c"
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
    PRIV_REQUIRES 
        driver
        esp_event
)
