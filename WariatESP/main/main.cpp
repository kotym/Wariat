#include <stdio.h>
#include "WariatCommon/CommunicationProtocol.hpp"
#include "STM32Interface.hpp"
#include "driver/uart.h"

// Example initialization and usage
STM32Interface stm32(UART_NUM_2);

extern "C" void app_main(void)
{
    // Initialize the interface
    // Change TX/RX pins according to your connection
    stm32.init(17, 16); 

    // Example of sending a "move forward" command after 5 seconds
    vTaskDelay(5000 / portTICK_PERIOD_MS);
    WariatCommon::MovePayload move_payload = { .distance_mm = 5000 };
    stm32.sendCommand(WariatCommon::CommandType::CMD_MOVE_FORWARD, move_payload);

    // In a loop, receive and process events from STM32
    while (true)
    {
        stm32.receiveAndProcess();
        vTaskDelay(10 / portTICK_PERIOD_MS); // Short delay to avoid high CPU load
    }
}
