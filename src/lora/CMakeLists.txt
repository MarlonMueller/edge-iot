idf_component_register(
    SRCS "lora_package.c" "lora_esp32.c" "lora_esp32_logic.c"
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
    PRIV_REQUIRES 
        driver
        gps
)
