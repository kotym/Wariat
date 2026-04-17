#include "hFramework.h"
#include "WariatCommon/CommunicationProtocol.hpp"
#include "src/ESPInterface.hpp"

// Example main loop in hMain
void hMain()
{
    // Initialize UART for communication with ESP32
    ESPInterface esp(hExt.serial);
    esp.init();

    

    sys.delay(1000);

    // Example of sending an obstacle event
    WariatCommon::ObstaclePayload obstacle = { .distance_mm = 4000 };
    esp.sendEvent(WariatCommon::EventType::EVENT_OBSTACLE_DETECTED, obstacle);

    for (;;)
    {
        // In a loop, listen for and process commands from ESP32
        esp.receiveAndProcess();
        sys.delay(10);
    }
}
