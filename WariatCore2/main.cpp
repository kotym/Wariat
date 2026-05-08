#include "hFramework.h"
//#include "WariatCommon/CommunicationProtocol.hpp"
//#include "src/ESPInterface.hpp"
#include "src/COREInterface.hpp"
#include "src/Motors.hpp"
#include "src/COREHcSr04s.hpp"

// Example main loop in hMain
void hMain()
{
    // Initialize UART for communication with ESP32
    //ESPInterface esp(hExt.serial);
    //esp.init();
	sys.setLogDev(&Serial);
    Serial.printf("IstartI0 ");
    //Motors::Init();
    COREInterface::Init(hExt.serial);
    sys.taskCreate([](){ COREInterface::Get().ReceiveCommands(); });

    
    //COREInterface::Get().ReceiveCommands();
    COREHcSr04s hcSr04s;
    
    Serial.printf(" IstartI1 ");
    //hFramework::hSerial& uart(hExt.serial);
    //uart.init(11520);
    
    //sys.delay(1000);
    
    // Example of sending an obstacle event
    //WariatCommon::ObstaclePayload obstacle = { .distance_mm = 4000 };
    //esp.sendEvent(WariatCommon::EventType::EVENT_OBSTACLE_DETECTED, obstacle);
    hLED1.off();
    hLED2.off();
    hLED3.off();
    Serial.printf(" IstartI2 ");
    for (;;)
    {
        // In a loop, listen for and process commands from ESP32
        //esp.receiveAndProcess();
        Serial.printf(" . ");
        //uart.printf("asfasdf");
        sys.delay(5000);
    }
}
