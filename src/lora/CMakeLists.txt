idf_component_register(
    SRCS "lora.c"
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
    PRIV_REQUIRES driver
    )