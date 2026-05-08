#include <cstdint>

#include "CommunicationProtocol.hpp"
#include "IdleState.hpp"
#include "Mind.hpp"
#include "MoveForwardState.hpp"
#include "RobotStateContext.hpp"
#include "STM32Interface.hpp"
#include "WWW.hpp"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <esp_event.h>
#include <esp_log.h>
#include <esp_system.h>
#include <nvs_flash.h>
#include <sys/param.h>
#include "esp_netif.h"
#include "esp_eth.h"
#include "esp_wifi.h"
#include "protocol_examples_common.h"

STM32Interface stm32(UART_NUM_2);

static bool PublishCommand(void* userData,
                           WariatCommon::CommandType commandType,
                           const void* payload,
                           uint8_t payloadSize)
{
    auto* transport = static_cast<STM32Interface*>(userData);
    if (transport == nullptr)
    {
        return false;
    }

    if (payload == nullptr || payloadSize == 0)
    {
        return transport->sendCommand(commandType);
    }

    switch (commandType)
    {
        case WariatCommon::CommandType::CMD_MOVE_FORWARD:
        case WariatCommon::CommandType::CMD_MOVE_BACKWARD:
            if (payloadSize == sizeof(WariatCommon::MovePayload))
            {
                const auto* movePayload = static_cast<const WariatCommon::MovePayload*>(payload);
                return transport->sendCommand(commandType, *movePayload);
            }
            break;
        case WariatCommon::CommandType::CMD_TURN_LEFT:
        case WariatCommon::CommandType::CMD_TURN_RIGHT:
            if (payloadSize == sizeof(WariatCommon::TurnPayload))
            {
                const auto* turnPayload = static_cast<const WariatCommon::TurnPayload*>(payload);
                return transport->sendCommand(commandType, *turnPayload);
            }
            break;
        default:
            break;
    }

    return false;
}

static void disconnect_handler(void* arg, esp_event_base_t event_base,
                               int32_t event_id, void* event_data)
{
    WWW* www = static_cast<WWW*>(arg);
    www->StopServer();
}

static void connect_handler(void* arg, esp_event_base_t event_base,
                            int32_t event_id, void* event_data)
{
    WWW* www = static_cast<WWW*>(arg);
    www->StartServer();
}

extern "C" void app_main(void)
{
    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    /* Register event handlers to start server when Wi-Fi or Ethernet is connected,
     * and stop server when disconnection happens.
     */

    WWW www;

#ifdef CONFIG_EXAMPLE_CONNECT_WIFI
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &www));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &www));
#endif // CONFIG_EXAMPLE_CONNECT_WIFI
#ifdef CONFIG_EXAMPLE_CONNECT_ETHERNET
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_ETH_GOT_IP, &connect_handler, &www));
    ESP_ERROR_CHECK(esp_event_handler_register(ETH_EVENT, ETHERNET_EVENT_DISCONNECTED, &disconnect_handler, &www));
#endif // CONFIG_EXAMPLE_CONNECT_ETHERNET

    /* This helper function configures Wi-Fi or Ethernet, as selected in menuconfig.
     * Read "Establishing Wi-Fi or Ethernet Connection" section in
     * examples/protocols/README.md for more information about this function.
     */
    ESP_ERROR_CHECK(example_connect());

    // Change TX/RX pins according to your connection.
    stm32.init(17, 16);
    Mind mind;
    RobotStateContext context{};
    context.commandUserData = &stm32;
    context.sendCommandFn = &PublishCommand;
    context.currentTimeMs = static_cast<uint64_t>(esp_timer_get_time() / 1000ULL);

    mind.transitionTo<IdleState>(context);

    while (true)
    {
        stm32.receiveAndProcess();

        context.currentTimeMs = static_cast<uint64_t>(esp_timer_get_time() / 1000ULL);
        context.isObstacleDetected = stm32.consumeObstacleDetected();

        mind.update(context);
        mind.processRequestedTransition<IdleState, MoveForwardState>(context);

        vTaskDelay(10 / portTICK_PERIOD_MS);
    }
}
