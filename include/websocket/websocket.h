/**
 * @file websocket.h
 * @brief Websocket functions
 *
 */

#pragma once

#include <sys/socket.h>

typedef struct esp_websocket_client *esp_websocket_client_handle_t;

#ifdef __cplusplus
extern "C"
{
#endif

    /**
     * @brief Connect to websocket
     *
     * @return Client handle
     */
    esp_websocket_client_handle_t websocket_connect(void);

    /**
     * @brief Send data to websocket
     *
     * @param client [in] Client handle
     * @param data [in] Data to send
     * @param len [in] Length of data
     *
     * @pre client must be initialized before calling this function.
     */
    void websocket_send(esp_websocket_client_handle_t client, char *data, size_t len);

    /**
     * @brief Disconnect from websocket
     *
     * @param client [in] Client handle
     */
    void websocket_disconnect(esp_websocket_client_handle_t client);

#ifdef __cplusplus
}
#endif
