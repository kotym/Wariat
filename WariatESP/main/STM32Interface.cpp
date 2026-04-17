#include "STM32Interface.hpp"
#include "WariatCommon/CommunicationProtocol.hpp"

#include "esp_log.h"
#include <cstring>

static const char* TAG = "STM32_INTERFACE";

using namespace WariatCommon;

STM32Interface::STM32Interface(uart_port_t uart_num)
	: uart_num(uart_num)
{
}

void STM32Interface::init(int tx_pin, int rx_pin, uint32_t baudrate)
{
	uart_config_t uart_config = {
		.baud_rate = (int)baudrate,
		.data_bits = UART_DATA_8_BITS,
		.parity = UART_PARITY_DISABLE,
		.stop_bits = UART_STOP_BITS_1,
		.flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
		.source_clk = UART_SCLK_DEFAULT,
	};
	ESP_ERROR_CHECK(uart_driver_install(uart_num, 2048, 2048, 0, NULL, 0));
	ESP_ERROR_CHECK(uart_param_config(uart_num, &uart_config));
	ESP_ERROR_CHECK(uart_set_pin(uart_num, tx_pin, rx_pin, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}

// --- Implementation of methods from DeviceInterface ---

int STM32Interface::sendBytes(const void* data, size_t len)
{
	return uart_write_bytes(uart_num, data, len);
}

int STM32Interface::receiveBytes(void* data, size_t len, uint32_t timeout)
{
	return uart_read_bytes(uart_num, data, len, timeout / portTICK_PERIOD_MS);
}

uint32_t STM32Interface::availableBytes() const
{
	size_t available;
	uart_get_buffered_data_len(uart_num, &available);
	return available;
}

// --- Protocol Methods ---

bool STM32Interface::sendFrame(uint8_t msgType)
{
    Frame<char> frame; // Use char as an "empty" payload
    frame.header.sof = SOF_VALUE;
    frame.header.type = msgType;
    frame.header.payloadSize = 0;
    frame.checksum = calculateChecksum(&frame, sizeof(FrameHeader));

    return sendBytes(&frame, sizeof(FrameHeader) + 1) == sizeof(FrameHeader) + 1;
}

bool STM32Interface::sendCommand(CommandType cmdType)
{
    return sendFrame(static_cast<uint8_t>(cmdType));
}

bool STM32Interface::receiveAndProcess()
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
		ESP_LOGE(TAG, "Checksum error!");
		return false;
	}

	// 5. Process the event
	handleEvent(static_cast<EventType>(header.type), payload, header.payloadSize);

	return true;
}

bool STM32Interface::consumeObstacleDetected()
{
	const bool wasDetected = obstacleDetected;
	obstacleDetected = false;
	return wasDetected;
}

void STM32Interface::handleEvent(EventType event, const uint8_t* payload, uint8_t payloadSize)
{
	// Application logic reacts to events from STM32 here
	switch (event)
	{
		case EventType::EVENT_OBSTACLE_DETECTED:
		{
			if (payloadSize == sizeof(ObstaclePayload))
			{
				const auto* obstacleData = reinterpret_cast<const ObstaclePayload*>(payload);
				obstacleDetected = true;
				ESP_LOGI(TAG, "Obstacle detected at %lu mm", obstacleData->distance_mm);
				// Update map, plan new path, etc.
			}
			break;
		}
		case EventType::EVENT_STATUS_REPORT:
		{
			if (payloadSize == sizeof(StatusPayload))
			{
				const auto* statusData = reinterpret_cast<const StatusPayload*>(payload);
				ESP_LOGI(TAG, "Status: pos=(%ld, %ld), angle=%.2f, battery=%u mV",
					statusData->position_x_mm, statusData->position_y_mm,
					statusData->angle_degs_x100 / 100.0f, statusData->battery_mv);
			}
			break;
		}
		default:
			ESP_LOGW(TAG, "Unknown event type: 0x%02X", static_cast<uint8_t>(event));
			break;
	}
}
