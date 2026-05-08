#pragma once
#include "esp_err.h"
#include "esp_netif_types.h"

#ifdef __cplusplus
extern "C" {
#endif

// Simple WiFi connection function - replaces protocol_examples_common
esp_err_t example_connect(void);

#ifdef __cplusplus
}
#endif