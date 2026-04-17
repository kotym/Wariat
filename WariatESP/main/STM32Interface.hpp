#pragma once

#include "WariatCommon/DeviceInterface.hpp"
#include "WariatCommon/CommunicationProtocol.hpp"

#include "driver/uart.h"

class STM32Interface : public WariatCommon::DeviceInterface<STM32Interface>
{
public:
	explicit STM32Interface(uart_port_t uart_num);

	void init(int tx_pin, int rx_pin, uint32_t baudrate = 115200);

	// --- Implementation of methods for DeviceInterface ---
	int sendBytes(const void* data, size_t len);
	int receiveBytes(void* data, size_t len, uint32_t timeout);
	uint32_t availableBytes() const;

	// --- High-level methods for sending commands ---
	template <typename T>
	bool sendCommand(WariatCommon::CommandType cmdType, const T& payload);
	bool sendCommand(WariatCommon::CommandType cmdType); // For commands without a payload

	// --- Methods for receiving and parsing events ---
	bool receiveAndProcess();
	bool consumeObstacleDetected();

private:
	uart_port_t uart_num;

	// Private methods for protocol handling
	template <typename T>
	bool sendFrame(uint8_t msgType, const T& payload);
	bool sendFrame(uint8_t msgType);

	void handleEvent(WariatCommon::EventType event, const uint8_t* payload, uint8_t payloadSize);

	bool obstacleDetected = false;
};

// Template implementation must be in the header file
template <typename T>
bool STM32Interface::sendFrame(uint8_t msgType, const T& payload)
{
	WariatCommon::Frame<T> frame;
	frame.header.sof = WariatCommon::SOF_VALUE;
	frame.header.type = msgType;
	frame.header.payloadSize = sizeof(T);
	frame.payload = payload;
	frame.checksum = WariatCommon::calculateChecksum(&frame, sizeof(frame) - sizeof(frame.checksum));

	return sendBytes(&frame, sizeof(frame)) == sizeof(frame);
}

template <typename T>
bool STM32Interface::sendCommand(WariatCommon::CommandType cmdType, const T& payload)
{
	return sendFrame(static_cast<uint8_t>(cmdType), payload);
}
