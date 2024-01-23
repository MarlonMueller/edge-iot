idf_build_get_property(target IDF_TARGET)

set(srcs
        main.cpp
        ../model/birdnet_default_coefficient.cpp
)

set(src_dirs        
        ../model
)

set(include_dirs    
        ../model
        ../esp-dl/include
        ../esp-dl/include/tool
        ../esp-dl/include/typedef
        ../esp-dl/include/nn
        ../esp-dl/include/layer
)

set(requires
        esp-led
        # wifi
        audio
        esp-dl
        # websocket
        heap-log
)

idf_component_register(
    SRCS ${srcs} 
    SRC_DIRS ${src_dirs} 
    INCLUDE_DIRS ${include_dirs} 
    REQUIRES ${requires}
    PRIV_REQUIRES 
        driver
)

set(lib     libdl.a)

if(${IDF_TARGET} STREQUAL "esp32")
    set(links   "-L ${CMAKE_CURRENT_SOURCE_DIR}/../esp-dl/lib/esp32")

elseif(${IDF_TARGET} STREQUAL "esp32s2")
    set(links   "-L ${CMAKE_CURRENT_SOURCE_DIR}/../esp-dl/lib/esp32s2")

elseif(${IDF_TARGET} STREQUAL "esp32s3")
    set(links   "-L ${CMAKE_CURRENT_SOURCE_DIR}/../esp-dl/lib/esp32s3")

elseif(${IDF_TARGET} STREQUAL "esp32c3")    
    set(links   "-L ${CMAKE_CURRENT_SOURCE_DIR}/../esp-dl/lib/esp32c3")

endif()


target_link_libraries(${COMPONENT_TARGET} ${links})
target_link_libraries(${COMPONENT_TARGET} ${lib})
