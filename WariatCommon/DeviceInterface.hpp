#pragma once

#include <cstddef>
#include <cstdint>

namespace WariatCommon
{

/**
 * @brief Base class for communication interfaces using CRTP to avoid vtables.
 *
 * This class uses the Curiously Recurring Template Pattern (CRTP) to achieve
 * static polymorphism, eliminating the overhead of virtual functions on embedded targets.
 *
 * @tparam Derived The actual implementation class (e.g., ESPInterface, STM32Interface).
 */
template <typename Derived>
class DeviceInterface
{
public:
	/**
	 * @brief Sends a specified number of bytes.
	 * @param data Pointer to the buffer with data to send.
	 * @param len Number of bytes to send.
	 * @return The number of bytes sent, or a negative value on error.
	 */
	int sendBytes(const void* data, size_t len)
	{
		return static_cast<Derived*>(this)->sendBytes(data, len);
	}

	/**
	 * @brief Receives a specified number of bytes.
	 * @param data Pointer to the buffer where received data will be stored.
	 * @param len Number of bytes to receive.
	 * @param timeout Timeout in milliseconds.
	 * @return The number of bytes received, or a negative value on error/timeout.
	 */
	int receiveBytes(void* data, size_t len, uint32_t timeout)
	{
		return static_cast<Derived*>(this)->receiveBytes(data, len, timeout);
	}

	/**
	 * @brief Returns the number of bytes available for reading.
	 * @return The number of available bytes.
	 */
	uint32_t availableBytes() const
	{
		return static_cast<const Derived*>(this)->availableBytes();
	}
};

} // namespace WariatCommon
