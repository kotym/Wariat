#include "ESPInterface.hpp"

#include "hFramework.h"

#include <algorithm>
#include <cstring>

using namespace hFramework;

ESPInterface::ESPInterface(hSerial& uart)
	: uart(uart)
{
}

void ESPInterface::init(uint32_t baudrate, Parity parity, StopBits stopBits)
{
	uart.init(baudrate, parity, stopBits);
}

int ESPInterface::send(const void* data, std::size_t len, uint32_t timeout)
{
	return uart.write(data, static_cast<int>(len), timeout);
}

int ESPInterface::sendText(const char* text, uint32_t timeout)
{
	if (text == nullptr)
	{
		return 0;
	}

	return send(text, std::strlen(text), timeout);
}

int ESPInterface::sendLine(const char* line, uint32_t timeout)
{
	int written = sendText(line, timeout);
	if (written < 0)
	{
		return written;
	}

	const char eol[] = "\r\n";
	const int eolWritten = uart.write(eol, sizeof(eol) - 1, timeout);
	if (eolWritten < 0)
	{
		return eolWritten;
	}

	return written + eolWritten;
}

int ESPInterface::receive(void* data, std::size_t len, uint32_t timeout)
{
	return uart.read(data, static_cast<int>(len), timeout);
}

int ESPInterface::receiveAvailable(void* data, std::size_t maxLen)
{
	const uint32_t waiting = uart.available();
	if (waiting == 0 || maxLen == 0)
	{
		return 0;
	}

	const std::size_t toRead = std::min<std::size_t>(maxLen, waiting);
	return uart.read(data, static_cast<int>(toRead), 0);
}

bool ESPInterface::receiveLine(char* out, std::size_t outSize, uint32_t timeout)
{
	if (out == nullptr || outSize == 0)
	{
		return false;
	}

	std::size_t index = 0;
	while (index + 1 < outSize)
	{
		char c = '\0';
		const int readCount = uart.read(&c, 1, timeout);
		if (readCount != 1)
		{
			break;
		}

		if (c == '\n')
		{
			break;
		}

		if (c == '\r')
		{
			continue;
		}

		out[index++] = c;
	}

	out[index] = '\0';
	return index > 0;
}

uint32_t ESPInterface::available() const
{
	return uart.available();
}

void ESPInterface::flushRx()
{
	uart.flushRx();
}