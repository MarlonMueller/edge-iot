idf_component_register(
    SRCS "microphone.cpp" "preprocess.cpp"
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
    PRIV_REQUIRES
        esp-dsp
        driver
)