#include <cstdint>

#include "CommunicationProtocol.hpp"
#include "IdleState.hpp"
#include "Mind.hpp"
#include "MoveForwardState.hpp"
#include "RobotStateContext.hpp"
#include "STM32Interface.hpp"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

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

extern "C" void app_main(void)
{
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
