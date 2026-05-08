#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "driver/uart.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "ComProtocol.hpp"
#include "ESPWariat.hpp"

static const char *TAG = "uart_events";
#define RX_BUF_SIZE 256
#define TX_BUF_SIZE 256
#define UART_EVENT_TASK_STACK 4096
// TODO this buffers can be shrunk
static QueueHandle_t uart3_queue;

static void uart_event_task(void *pvParameters)
{
    uart_event_t event;
    // size_t buffered_size;
    uint8_t *readData = (uint8_t *)malloc(RX_BUF_SIZE);
    assert(readData);

    while (true)
    {
        if (xQueueReceive(uart3_queue, (void *)&event, portMAX_DELAY))
        {
            memset(readData, 0, RX_BUF_SIZE);
            switch (event.type)
            {
            case UART_DATA:
            {
                if (event.size > RX_BUF_SIZE)
                {
                    ESP_LOGD(TAG, "MyUart3: event > RX_BUF_SIZE");
                    break;
                }

                const int readSize = uart_read_bytes(UART_NUM_2, readData, event.size, pdMS_TO_TICKS(10));
                if (readSize <= 0)
                {
                    ESP_LOGD(TAG, "MyUart3: empty read");
                    break;
                }
                
                if (readSize != event.size)
                {
                    ESP_LOGD(TAG, "MyUart3: readSize != event.size");
                    break;
                }

                uint8_t* data = readData;
                const uint8_t* startSearchEnd = readData + readSize - (1 + 1 + 1); // payloadType + checkSum + packetEnd
                while (*data != 0xAA && data < startSearchEnd) {
                    ++data;
                }

                if (data >= startSearchEnd)
                {
                    ESP_LOGD(TAG, "MyUart3: no frame start or packet to small");
                    break;
                }

                const WariatCommon::PacketPayloadType payloadType = *(WariatCommon::PacketPayloadType*)(++data);
                const uint8_t payloadSize = GetPayloadSize(payloadType);
                if (data + payloadSize >= readData + readSize - (1 + 1)) // checkSum + packetEnd
                {
                    ESP_LOGD(TAG, "MyUart3: short frame");
                    break;
                }

                void* const payload = ++data;
                const uint8_t checkSum = *(data += payloadSize);
                if (*(data + 1) != 0xFA || checkSum != CalcCheckSum(payloadType, (uint8_t*)payload, payloadSize))
                {
                    ESP_LOGD(TAG, "MyUart3: checksum/end mismatch");
                    break;
                }

                // Process event
                auto w = WariatESP::Get();
                w.ProcessEvent(payloadType, payload);
            }
            break;
            case UART_FIFO_OVF:
                ESP_LOGD(TAG, "MyUart3 FIFO_OVF");
                uart_flush_input(UART_NUM_2);
                xQueueReset(uart3_queue);
                break;
            case UART_BUFFER_FULL:
                ESP_LOGD(TAG, "MyUart3 UART_BUFFER_FULL");
                uart_flush_input(UART_NUM_2);
                xQueueReset(uart3_queue);
                break;
            case UART_BREAK:
                // RX line can generate frequent BREAK events when floating.
                // Keep log level low to avoid starving CPU and flooding monitor.
                ESP_LOGD(TAG, "MyUart3 UART_BREAK");
                break;
            case UART_PARITY_ERR:
                ESP_LOGD(TAG, "MyUart3 UART_PARITY_ERR");
                break;
            case UART_FRAME_ERR:
                ESP_LOGD(TAG, "MyUart3 UART_FRAME_ERR");
                break;
            default:
                ESP_LOGD(TAG, "MyUart3 event type: %d", event.type);
                break;
            }

            // Under noisy RX conditions this task can run continuously and starve IDLE0.
            vTaskDelay(pdMS_TO_TICKS(1));
        }
    }
    free(readData);
    readData = nullptr;
    vTaskDelete(nullptr);
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
    // uart_intr_config_t uart_intr = {
    //    .intr_enable_mask = UART_RXFIFO_FULL_INT_ENA_M | UART_INTR_RXFIFO_TOUT,
    //    .rxfifo_full_thresh = 100,
    //    .rx_timeout_thresh = 10,
    //};
    // ESP_ERROR_CHECK(uart_intr_config(uart_num, &uart_intr));
    // Enable UART RX FIFO full threshold and timeout interrupts
    // ESP_ERROR_CHECK(uart_enable_rx_intr(UART_NUM_2));

    ESP_ERROR_CHECK(uart_driver_install(UART_NUM_2, RX_BUF_SIZE, TX_BUF_SIZE, 5, &uart3_queue, 0));
    uart_param_config(UART_NUM_2, &uart_config);
    uart_set_pin(UART_NUM_2, GPIO_NUM_16 /*tx*/, GPIO_NUM_17 /*rx*/, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE);
    gpio_set_pull_mode(GPIO_NUM_17, GPIO_PULLUP_ONLY);

    xTaskCreate(uart_event_task, "uart_event_task", UART_EVENT_TASK_STACK, nullptr, 10, nullptr);
}