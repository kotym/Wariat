#pragma once
#include "Wariat.hpp"
#include "Map.hpp"
#include "UartInterface.hpp"
#include "ESPMapRenderer.hpp"


class WariatESP : public WariatCommon::Wariat<Map, ESPMapRenderer, UartInterface>
{
public:
    WariatESP()
    {
        // lewy
        hcSr04Offsets[0].position.x = -4.7f;
        hcSr04Offsets[0].position.y = 7.7f;
        hcSr04Offsets[0].rotation = M_PI_2;

        // przod lewy
        hcSr04Offsets[1].position.x = 4.f;
        hcSr04Offsets[1].position.y = 3.f;
        hcSr04Offsets[1].rotation = M_PI / 12.f;

        // przod prawy
        hcSr04Offsets[2].position.x = 4.f;
        hcSr04Offsets[2].position.y = -3.f;
        hcSr04Offsets[2].rotation = -M_PI / 12.f;

        // prawy
        hcSr04Offsets[3].position.x = -4.7f;
        hcSr04Offsets[3].position.y = -7.7f;
        hcSr04Offsets[3].rotation = -M_PI_2;
    }
};