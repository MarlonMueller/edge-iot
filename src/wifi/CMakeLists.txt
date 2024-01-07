idf_component_register(
    SRCS "wifi.c"
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
    PRIV_REQUIRES esp_wifi nvs_flash
    )