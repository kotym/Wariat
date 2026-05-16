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
                    //Serial.printf(" HCSR04: id: %d dist: %d \n", id, distance);
                    if (distance <= 0) continue;
                    WariatCommon::Payload::HcSr04Reading readingPayload(id, distance);
                    COREInterface::Get().SendEvent(readingPayload);
                    sys.delay_ms(100);
                } 
            });
    }

    uint8_t lastRead = 0;


    hModules::DistanceSensor hcSr04s[4];
};