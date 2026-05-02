#include <cstdint>

//#include "AutonomusState.hpp"
//#include "CommunicationProtocol.hpp"
//#include "IdleState.hpp"
//#include "ManualState.hpp"
//#include "Mind.hpp"
//#include "RobotStateContext.hpp"
//#include "RobotStateOperations.hpp"
//#include "STM32Interface.hpp"
//#include "WorkingState.hpp"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

#include "Wariat.hpp"
void InitUart();

// STM32Interface stm32(UART_NUM_2);

// static bool PublishCommand(void* userData,
//                            WariatCommon::CommandType commandType,
//                            const void* payload,
//                            uint8_t payloadSize)
// {
//     auto* transport = static_cast<STM32Interface*>(userData);
//     if (transport == nullptr)
//     {
//         return false;
//     }

//     if (payload == nullptr || payloadSize == 0)
//     {
//         return transport->sendCommand(commandType);
//     }

//     switch (commandType)
//     {
//         case WariatCommon::CommandType::CMD_MOVE_FORWARD:
//         case WariatCommon::CommandType::CMD_MOVE_BACKWARD:
//             if (payloadSize == sizeof(WariatCommon::MovePayload))
//             {
//                 const auto* movePayload = static_cast<const WariatCommon::MovePayload*>(payload);
//                 return transport->sendCommand(commandType, *movePayload);
//             }
//             break;
//         case WariatCommon::CommandType::CMD_TURN_LEFT:
//         case WariatCommon::CommandType::CMD_TURN_RIGHT:
//             if (payloadSize == sizeof(WariatCommon::TurnPayload))
//             {
//                 const auto* turnPayload = static_cast<const WariatCommon::TurnPayload*>(payload);
//                 return transport->sendCommand(commandType, *turnPayload);
//             }
//             break;
//         default:
//             break;
//     }

//     return false;
// }

// static void UpdateSensors(RobotStateContext& context)
// {
//     auto* transport = static_cast<STM32Interface*>(context.commandUserData);
//     if (transport == nullptr)
//     {
//         return;
//     }

//     transport->receiveAndProcess();
//     context.isObstacleDetected = transport->consumeObstacleDetected();
// }

// static void UpdateMap(RobotStateContext& context)
// {
//     (void)context;
// }

// static void UpdateInput(RobotStateContext& context)
// {
//     (void)context;
// }

// static void UpdateNavigation(RobotStateContext& context)
// {
//     (void)context;
// }

extern "C" void app_main(void)
{
    InitUart();
    // Change TX/RX pins according to your connection.
    // stm32.init(17, 16);

    // Mind mind;
    // static const RobotStateOperations operations{
    //     .updateSensors = &UpdateSensors,
    //     .updateMap = &UpdateMap,
    //     .updateInput = &UpdateInput,
    //     .updateNavigation = &UpdateNavigation};

    // RobotStateContext context{};
    // context.commandUserData = &stm32;
    // context.sendCommandFn = &PublishCommand;
    // context.operations = &operations;
    // context.currentTimeMs = static_cast<uint64_t>(esp_timer_get_time() / 1000ULL);

    // mind.transitionTo<IdleState>(context);

    // while (true)
    // {
    //     context.currentTimeMs = static_cast<uint64_t>(esp_timer_get_time() / 1000ULL);

    //     mind.update(context);
    //     (void)mind.processRequestedTransitionWithBindings<
    //         Mind::StateBinding<MindStateId::Idle, IdleState>,
    //         Mind::StateBinding<MindStateId::Working, WorkingState>,
    //         Mind::StateBinding<MindStateId::Manual, ManualState>,
    //         Mind::StateBinding<MindStateId::Autonomus, AutonomusState>>(context);

    //     vTaskDelay(10 / portTICK_PERIOD_MS);
    // }
}
