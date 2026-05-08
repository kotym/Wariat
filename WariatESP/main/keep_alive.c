#include "keep_alive.h"
#include <string.h>
#include <sys/param.h>
#include <esp_log.h>

static const char *TAG = "keep_alive";

struct wss_keep_alive_storage {
    wss_keep_alive_config_t config;
    httpd_handle_t hd;
    int *client_fds;
    size_t client_fds_size;
    SemaphoreHandle_t lock;
    TaskHandle_t task_handle;
};

static void keep_alive_task(void *arg)
{
    wss_keep_alive_t h = (wss_keep_alive_t) arg;
    while (1) {
        vTaskDelay(h->config.keep_alive_period_ms / portTICK_PERIOD_MS);
        xSemaphoreTake(h->lock, portMAX_DELAY);
        for (size_t i = 0; i < h->client_fds_size; ++i) {
            int fd = h->client_fds[i];
            if (fd >= 0) {
                if (h->config.check_client_alive_cb) {
                    if (!h->config.check_client_alive_cb(h, fd)) {
                        ESP_LOGW(TAG, "Client %d not alive, removing from list", fd);
                        h->client_fds[i] = -1;
                    }
                }
            }
        }
        xSemaphoreGive(h->lock);
    }
    vTaskDelete(NULL);
}

wss_keep_alive_t wss_keep_alive_start(wss_keep_alive_config_t *config)
{
    wss_keep_alive_t h = calloc(1, sizeof(struct wss_keep_alive_storage));
    if (!h) {
        ESP_LOGE(TAG, "Failed to allocate memory for keep alive");
        return NULL;
    }
    memcpy(&h->config, config, sizeof(wss_keep_alive_config_t));
    h->client_fds = malloc(sizeof(int) * config->max_clients);
    if (!h->client_fds) {
        ESP_LOGE(TAG, "Failed to allocate memory for client fds");
        free(h);
        return NULL;
    }
    memset(h->client_fds, -1, sizeof(int) * config->max_clients);
    h->client_fds_size = config->max_clients;
    h->lock = xSemaphoreCreateMutex();
    if (!h->lock) {
        ESP_LOGE(TAG, "Failed to create mutex");
        free(h->client_fds);
        free(h);
        return NULL;
    }
    if (xTaskCreate(&keep_alive_task, "keep_alive_task", 4096, h, 5, &h->task_handle) != pdPASS) {
        ESP_LOGE(TAG, "Failed to create keep alive task");
        vSemaphoreDelete(h->lock);
        free(h->client_fds);
        free(h);
        return NULL;
    }
    return h;
}

esp_err_t wss_keep_alive_stop(wss_keep_alive_t h)
{
    if (!h) {
        return ESP_ERR_INVALID_ARG;
    }
    vTaskDelete(h->task_handle);
    vSemaphoreDelete(h->lock);
    free(h->client_fds);
    free(h);
    return ESP_OK;
}

esp_err_t wss_keep_alive_add_client(wss_keep_alive_t h, int fd)
{
    if (!h || fd < 0) {
        return ESP_ERR_INVALID_ARG;
    }
    xSemaphoreTake(h->lock, portMAX_DELAY);
    for (size_t i = 0; i < h->client_fds_size; ++i) {
        if (h->client_fds[i] == -1) {
            h->client_fds[i] = fd;
            xSemaphoreGive(h->lock);
            return ESP_OK;
        }
    }
    xSemaphoreGive(h->lock);
    return ESP_ERR_NO_MEM;
}

esp_err_t wss_keep_alive_remove_client(wss_keep_alive_t h, int fd)
{
    if (!h || fd < 0) {
        return ESP_ERR_INVALID_ARG;
    }
    xSemaphoreTake(h->lock, portMAX_DELAY);
    for (size_t i = 0; i < h->client_fds_size; ++i) {
        if (h->client_fds[i] == fd) {
            h->client_fds[i] = -1;
            xSemaphoreGive(h->lock);
            return ESP_OK;
        }
    }
    xSemaphoreGive(h->lock);
    return ESP_ERR_NOT_FOUND;
}

bool wss_keep_alive_client_is_active(wss_keep_alive_t h, int fd)
{
    if (!h || fd < 0) {
        return false;
    }
    xSemaphoreTake(h->lock, portMAX_DELAY);
    for (size_t i = 0; i < h->client_fds_size; ++i) {
        if (h->client_fds[i] == fd) {
            xSemaphoreGive(h->lock);
            return true;
        }
    }
    xSemaphoreGive(h->lock);
    return false;
}

void wss_keep_alive_set_user_ctx(wss_keep_alive_t h, void *ctx)
{
    if (h) {
        h->hd = (httpd_handle_t) ctx;
    }
}

void *wss_keep_alive_get_user_ctx(wss_keep_alive_t h)
{
    return h ? h->hd : NULL;
}