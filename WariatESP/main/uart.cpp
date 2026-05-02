#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "esp_log.h"
//#include "esp_intr_types.h"
#include "ComProtocol.hpp"
//#include "EventDispatcher.hpp"
#include "ESPWariat.hpp"

using namespace WariatCommon;


static const char *TAG = "uart_events";
#define RX_BUF_SIZE 128
#define TX_BUF_SIZE 128
static QueueHandle_t uart0_queue;

static void uart_event_task(void* pvParameters)
{
    uart_event_t event;
    //size_t buffered_size;
    uint8_t* dtmp = (uint8_t*)malloc(RX_BUF_SIZE);
    assert(dtmp);

    //uint8_t header;
    //uint8_t payload[32];

    while(true)
    {
        if(xQueueReceive(uart0_queue, (void*)&event, portMAX_DELAY))
        {
            memset(dtmp, 0, RX_BUF_SIZE);
            ESP_LOGI(TAG, "uart0 event:");
            switch(event.type)
            {
                case UART_DATA:
                {
                    uart_read_bytes(UART_NUM_0, dtmp, event.size, 10);
                    uint8_t* data = dtmp;
                    while (*data != 0xAA && data - dtmp < event.size);
                    if (data - dtmp >= event.size - 1) 
                    {
                        ESP_LOGI(TAG, "Error: uart0 event corrupted!!!");
                        break;
                    }
                    const PacketPayloadType payloadType = *(PacketPayloadType*)(++data);
                    const int32_t dataSize = event.size - (data - dtmp);
                    const uint8_t payloadSize = GetPayloadSize(payloadType);
                    if (payloadSize > dataSize)
                    {
                        ESP_LOGI(TAG, "Error: uart0 event corrupted!!!");
                        break;
                    }
                    void* payload = ++data;
                    uint8_t checkSum = *(data += payloadSize);
                    if (*(data + 1) != 0xFA || checkSum != CalcCheckSum(payloadType, (uint8_t*)payload, payloadSize))
                    {
                        ESP_LOGI(TAG, "Error: uart0 event corrupted!!!");
                        break;
                    }
                    // Process event
                    //EventDispatcher::ProcessEvent(payloadType, payload);
                    auto w = WariatESP::Get();
                    w.ProcessEvent(payloadType, payload);
                    
                    // while(uart_read_bytes(UART_NUM_1, &header, 1, 0) > 0) {
                    // if(header == 0xAA) {
                    //     // 2. Znaleźliśmy start ramki! Teraz czytamy ID i długość
                    //     uint8_t info[2]; 
                    //     uart_read_bytes(UART_NUM_1, info, 2, pdMS_TO_TICKS(10));
                    //     uint8_t cmd_id = info[0];
                    //     uint8_t len = info[1];

                    //     // 3. Czytamy resztę (payload + CRC)
                    //     if (len <= 32) {
                    //         uart_read_bytes(UART_NUM_1, payload, len + 1, pdMS_TO_TICKS(10));
                    //         // Tutaj weryfikujesz CRC i wykonujesz komendę
                    //         //procesuj_komende(cmd_id, payload, len);
                    //     }
                    // }
                }
                    break;
                case UART_FIFO_OVF:
                    ESP_LOGI(TAG, "uart0 FIFO_OVF");
                    uart_flush_input(UART_NUM_0);
                    xQueueReset(uart0_queue);
                    break;
                case UART_BUFFER_FULL:
                    ESP_LOGI(TAG, "uart0 UART_BUFFER_FULL");
                    uart_flush_input(UART_NUM_0);
                    xQueueReset(uart0_queue);
                    break;
                case UART_BREAK:
                    ESP_LOGI(TAG, "uart0 UART_BREAK");
                    break;
                case UART_PARITY_ERR:
                    ESP_LOGI(TAG, "uart0 UART_PARITY_ERR");
                    break;
                case UART_FRAME_ERR:
                    ESP_LOGI(TAG, "uart0 UART_FRAME_ERR");
                    break;
                default:
                    ESP_LOGI(TAG, "uart0 event type: %d", event.type);
                    break;
            }
        }
    }
    free(dtmp);
    dtmp = nullptr;
    vTaskDelete(nullptr);
}

template<class T>
void SendEvent(Packet<T> packet)
{
    uart_write_bytes(UART_NUM_0, &packet, sizeof(packet));
}

void InitUart()
{
    esp_log_level_set(TAG, ESP_LOG_INFO);

    uart_config_t uart_config = {
        .baud_rate = 115200,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
        
    };

    // TODO what to do with this, is it needed
    // Configure a UART interrupt threshold and timeout
    //uart_intr_config_t uart_intr = {
    //    .intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M | UART_INTR_RXFIFO_TOUT,
    //    .rxfifo_full_thresh = 100,
    //    .rx_timeout_thresh = 10,
    //};
    //ESP_ERROR_CHECK(uart_intr_config(uart_num, &uart_intr));
    // Enable UART RX FIFO full threshold and timeout interrupts
    //ESP_ERROR_CHECK(uart_enable_rx_intr(UART_NUM_0));

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_0, RX_BUF_SIZE, TX_BUF_SIZE, 5, &uart0_queue, 0));
    uart_param_config(UART_NUM_0, &uart_config);
    uart_set_pin(UART_NUM_0, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);

    xTaskCreate(uart_event_task, "uart_event_task", 2048, nullptr, 10, nullptr);
}