idf_component_register(
    SRCS "gps.c"
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
    PRIV_REQUIRES 
        driver
    REQUIRES
        esp_event
)
