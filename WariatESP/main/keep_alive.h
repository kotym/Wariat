#pragma once

#include <esp_https_server.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/semphr.h>
#include <esp_log.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct wss_keep_alive_storage *wss_keep_alive_t;

typedef struct {
    size_t max_clients;
    uint32_t keep_alive_period_ms;
    uint32_t not_alive_after_ms;
    bool (*client_not_alive_cb)(wss_keep_alive_t h, int fd);
    bool (*check_client_alive_cb)(wss_keep_alive_t h, int fd);
} wss_keep_alive_config_t;

#define KEEP_ALIVE_CONFIG_DEFAULT() { \
    .max_clients = 4, \
    .keep_alive_period_ms = 10000, \
    .not_alive_after_ms = 120000, \
    .client_not_alive_cb = NULL, \
    .check_client_alive_cb = NULL \
}

wss_keep_alive_t wss_keep_alive_start(wss_keep_alive_config_t *config);
esp_err_t wss_keep_alive_stop(wss_keep_alive_t h);
esp_err_t wss_keep_alive_add_client(wss_keep_alive_t h, int fd);
esp_err_t wss_keep_alive_remove_client(wss_keep_alive_t h, int fd);
bool wss_keep_alive_client_is_active(wss_keep_alive_t h, int fd);
void wss_keep_alive_set_user_ctx(wss_keep_alive_t h, void *ctx);
void *wss_keep_alive_get_user_ctx(wss_keep_alive_t h);

#ifdef __cplusplus
}
#endif