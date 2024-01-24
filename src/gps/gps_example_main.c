#include <stdio.h>
#include <stdlib.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "gps/gps.h"

void app_main(void)
{
    // Use the get_gps_data function to get data
    double latitude;
    double longitude;
    uint8_t hour;
    uint8_t minute;
    uint8_t second;
    bool valid;

    // Call the function to get GPS data
    get_gps_data(&latitude, &longitude, &hour, &minute, &second, &valid);

    // Now, you can use the extracted GPS data as needed
    printf("Valid: %d\n", valid);
    printf("Latitude: %.05f\n", latitude);
    printf("Longitude: %.05f\n", longitude);
    printf("Time: %02d:%02d:%02d\n", hour, minute, second);
}
