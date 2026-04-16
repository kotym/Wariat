#pragma once

#include <cstddef>
#include <cstdint>

namespace WariatCommon
{
// We use pragma pack to ensure the compiler does not add padding to the structure.
// This is crucial for binary consistency between different architectures (STM32 and ESP32).
#pragma pack(push, 1)

// Frame header, common to all messages
struct FrameHeader
{
	uint8_t sof;         // Start of Frame - a "magic" value, e.g., 0xFE
	uint8_t type;        // Message type (CommandType or EventType)
	uint8_t payloadSize; // Size of the data (payload)
};

// Full frame structure
template <typename T>
struct Frame
{
	FrameHeader header;
	T payload;
	uint8_t checksum;
};

#pragma pack(pop)

// --- Message Type Definitions ---

// Commands sent from ESP32 (Master) to STM32 (Slave)
enum class CommandType : uint8_t
{
	CMD_MOVE_FORWARD = 0x01,
	CMD_MOVE_BACKWARD = 0x02,
	CMD_TURN_LEFT = 0x03,
	CMD_TURN_RIGHT = 0x04,
	CMD_STOP = 0x05,
	CMD_GET_STATUS = 0x10,
};

// Events sent from STM32 (Slave) to ESP32 (Master)
enum class EventType : uint8_t
{
	EVENT_OBSTACLE_DETECTED = 0x81,
	EVENT_STATUS_REPORT = 0x90,
};

// --- Payload Struct Definitions ---

#pragma pack(push, 1)

// Payload for movement commands (e.g., move forward 5m)
// Distance in millimeters to avoid floating-point numbers
struct MovePayload
{
	uint32_t distance_mm;
};

// Payload for turn commands
// Angle in degrees * 100 to avoid float
struct TurnPayload
{
	int16_t angle_degs_x100;
};

// Payload for the obstacle detection event
struct ObstaclePayload
{
	uint32_t distance_mm;
};

// Payload for the status report
struct StatusPayload
{
	int32_t position_x_mm;
	int32_t position_y_mm;
	int16_t angle_degs_x100;
	uint16_t battery_mv;
};

#pragma pack(pop)

// --- Helper Functions ---

// Simple function to calculate the checksum (XOR)
inline uint8_t calculateChecksum(const void* data, std::size_t size)
{
	const uint8_t* bytes = static_cast<const uint8_t*>(data);
	uint8_t checksum = 0;
	for (std::size_t i = 0; i < size; ++i)
	{
		checksum ^= bytes[i];
	}
	return checksum;
}

constexpr uint8_t SOF_VALUE = 0xFE;

} // namespace WariatCommon
