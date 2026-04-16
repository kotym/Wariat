#pragma once

#include <cstddef>
#include <cstdint>
#include "hTypes.h"

namespace hFramework
{
class hSerial;
}

class ESPInterface
{
public:
	explicit ESPInterface(hFramework::hSerial& uart);

	void init(uint32_t baudrate = 115200,
			  Parity parity = Parity::None,
			  StopBits stopBits = StopBits::One);

	int send(const void* data, std::size_t len, uint32_t timeout = 0xffffffff);
	int sendText(const char* text, uint32_t timeout = 0xffffffff);
	int sendLine(const char* line, uint32_t timeout = 0xffffffff);

	int receive(void* data, std::size_t len, uint32_t timeout = 0xffffffff);
	int receiveAvailable(void* data, std::size_t maxLen);
	bool receiveLine(char* out, std::size_t outSize, uint32_t timeout = 1000);

	uint32_t available() const;
	void flushRx();

private:
	hFramework::hSerial& uart;

};