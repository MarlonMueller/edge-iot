idf_component_register(
    SRCS "microphone.c" "preprocess.c"
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
    PRIV_REQUIRES
        esp-dsp
        driver
)