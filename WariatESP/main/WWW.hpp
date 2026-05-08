#pragma once

#include <esp_https_server.h>
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>

class WWW
{
public:
    WWW();
    ~WWW();

    void Update(float deltaTime);
    void StartServer();
    void StopServer();
    void SendMapData(const uint8_t* data, size_t len);

private:
    httpd_handle_t server_;
    TaskHandle_t sendTaskHandle_;
    static const size_t max_clients_ = 4;

    static void sendTask(void* arg);
};