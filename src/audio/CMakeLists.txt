idf_component_register(
    SRCS "preprocess.c"
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
    PRIV_REQUIRES
        esp-dsp
)