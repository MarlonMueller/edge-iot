idf_component_register(
    SRCS "pc_main.c" "preprocess.cpp" "microphone.cpp"
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
    PRIV_REQUIRES
        esp-dsp
        driver
)