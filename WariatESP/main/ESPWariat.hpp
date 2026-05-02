#pragma once
#include "Wariat.hpp"
#include "Map.hpp"
#include "UartInterface.hpp"
#include "ESPMapRenderer.hpp"


class WariatESP : public WariatCommon::Wariat<Map, ESPMapRenderer, UartInterface>
{

};