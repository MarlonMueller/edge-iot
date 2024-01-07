idf_component_register(
    SRCS "websocket.c"
    INCLUDE_DIRS "${CMAKE_SOURCE_DIR}/include"
    PRIV_REQUIRES esp_websocket_client esp_event
    )