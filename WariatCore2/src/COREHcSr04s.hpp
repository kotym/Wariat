#pragma once
#include "hFramework.h"
#include "DistanceSensor.h"
#include "ComProtocol.hpp"
#include "COREInterface.hpp"

class COREHcSr04s
{
public:
    COREHcSr04s() : hcSr04s{hSens1, hSens2, hSens3, hSens4}
    {
        sys.taskCreate([&]()
            {
                while(true)
                {
                    int8_t id = ++lastRead % 4;
                    int16_t distance = hcSr04s[id].getDistance();
                    WariatCommon::Payload::HcSr04Reading readingPayload(id, distance);
                    COREInterface::Get().SendEvent(readingPayload);
                    Serial.printf(" send HCSR04: %d \n", distance);
                    sys.delay_ms(50);
                } 
            });
    }

    uint8_t lastRead = 0;


    hModules::DistanceSensor hcSr04s[4];
};