#include "ESPInterface.hpp"

#include "hFramework.h"
#include "WariatCommon/CommunicationProtocol.hpp"

#include <algorithm>
#include <cstring>

using namespace hFramework;
using namespace WariatCommon;

ESPInterface::ESPInterface(hSerial& uart)
	: uart(uart)
{
}

void ESPInterface::init(uint32_t baudrate, Parity parity, StopBits stopBits)
{
	uart.init(baudrate, parity, stopBits);
}

// --- Implementation of methods for DeviceInterface ---

int ESPInterface::sendBytes(const void* data, size_t len)
{
	return uart.write(data, static_cast<int>(len));
}

int ESPInterface::receiveBytes(void* data, size_t len, uint32_t timeout)
{
	return uart.read(data, static_cast<int>(len), timeout);
}

uint32_t ESPInterface::availableBytes() const
{
	return uart.available();
}

// --- Protocol Methods ---

bool ESPInterface::sendFrame(uint8_t msgType)
{
    Frame<char> frame; // Use char as an "empty" payload
    frame.header.sof = SOF_VALUE;
    frame.header.type = msgType;
    frame.header.payloadSize = 0;
    frame.checksum = calculateChecksum(&frame, sizeof(FrameHeader));

    return sendBytes(&frame, sizeof(FrameHeader) + 1) == sizeof(FrameHeader) + 1;
}

bool ESPInterface::sendEvent(EventType eventType)
{
    return sendFrame(static_cast<uint8_t>(eventType));
}

bool ESPInterface::receiveAndProcess()
{
	if (availableBytes() < sizeof(FrameHeader))
	{
		return false; // Not enough data for a header
	}

	FrameHeader header;
	uint8_t temp_sof;

	// 1. Search for Start of Frame (SOF)
	while (receiveBytes(&temp_sof, 1, 0) == 1)
	{
		if (temp_sof == SOF_VALUE)
		{
			break;
		}
	}

	if (temp_sof != SOF_VALUE)
	{
		return false; // SOF not found
	}

	header.sof = temp_sof;

	// 2. Receive the rest of the header
	if (receiveBytes(reinterpret_cast<uint8_t*>(&header) + 1, sizeof(header) - 1, 100) != sizeof(header) - 1)
	{
		return false; // Timeout or error
	}

	// 3. Receive payload and checksum
	uint8_t payload[255];
	uint8_t checksum_received;
	const size_t toRead = header.payloadSize + sizeof(checksum_received);

	if (receiveBytes(payload, toRead, 100) != toRead)
	{
		return false; // Failed to receive payload and checksum
	}
	
	checksum_received = payload[header.payloadSize];

	// 4. Verify checksum
	uint8_t checksum_calculated = calculateChecksum(&header, sizeof(header));
	checksum_calculated ^= calculateChecksum(payload, header.payloadSize);

	if (checksum_calculated != checksum_received)
	{
		// Checksum error, ignore frame
		return false;
	}

	// 5. Process the command
	handleCommand(static_cast<CommandType>(header.type), payload, header.payloadSize);

	return true;
}

void ESPInterface::handleCommand(CommandType cmd, const uint8_t* payload, uint8_t payloadSize)
{
	// Application logic reacts to commands here
	// E.g., control motors based on `cmd` and data from `payload`
	switch (cmd)
	{
		case CommandType::CMD_MOVE_FORWARD:
		{
			if (payloadSize == sizeof(MovePayload))
			{
				const auto* moveData = reinterpret_cast<const MovePayload*>(payload);
				// Call motor function: motors.moveForward(moveData->distance_mm);
				hFramework::hLED1.toggle(); // Example action
			}
			break;
		}
		case CommandType::CMD_STOP:
		{
			// motors.stop();
			hFramework::hLED2.toggle(); // Example action
			break;
		}
		// ... handle other commands
		default:
			// Unknown command
			break;
	}
}