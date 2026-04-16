#include "hFramework.h"
#include "src/ESPInterface.hpp"
#include <stddef.h>
#include <stdio.h>

using namespace hFramework;

void hMain()
{
	ESPInterface esp(hExt.serial);
	esp.init(115200, Parity::None, StopBits::One);
	Serial.init(460800, Parity::None, StopBits::One);
	esp.sendLine("STM32 ready");

	char rxBuf[64];
	for (;;)
	{
		const int rx = esp.receiveAvailable(rxBuf, sizeof(rxBuf) - 1);
		if (rx > 0)
		{
			rxBuf[rx] = '\0';
			Serial.printf("ESP -> STM32: %s\r\n", rxBuf);
		}

		sys.delay(10);
	}
}
