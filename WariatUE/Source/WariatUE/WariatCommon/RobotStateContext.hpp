#pragma once

#include <cstdint>
#include "CommunicationProtocol.hpp"
#include "MindStateId.hpp"

/**
 * @brief Holds all the data and interfaces the robot states need to operate.
 *
 * This context object is passed to each state, providing access to the robot's
 * hardware (or simulation equivalents) and shared data like maps or sensor readings.
 * This is a form of dependency injection, making the states independent of
 * global singletons and easier to test.
 */
struct RobotStateContext
{
    // Platform-provided callback used to publish commands.
    // `userData` can point to any transport implementation.
    using SendCommandFn = bool (*)(void* userData,
                                   WariatCommon::CommandType commandType,
                                   const void* payload,
                                   uint8_t payloadSize);

    void* commandUserData = nullptr;
    SendCommandFn sendCommandFn = nullptr;

    // Requested transition set by states and consumed by Mind.
    MindStateId requestedState = MindStateId::None;

    // --- Shared Data ---

    // The current time in milliseconds. Must be updated by the main loop on each platform.
    uint64_t currentTimeMs = 0;

    // Example of shared data
    bool isObstacleDetected = false;
    float targetAngle = 0.0f;
    float currentAngle = 0.0f;

    bool requestTransition(MindStateId nextState)
    {
        requestedState = nextState;
        return true;
    }

    bool publishCommand(WariatCommon::CommandType commandType)
    {
        if (sendCommandFn == nullptr)
        {
            return false;
        }
        return sendCommandFn(commandUserData, commandType, nullptr, 0);
    }

    template <typename TPayload>
    bool publishCommand(WariatCommon::CommandType commandType, const TPayload& payload)
    {
        if (sendCommandFn == nullptr)
        {
            return false;
        }
        return sendCommandFn(commandUserData,
                             commandType,
                             &payload,
                             static_cast<uint8_t>(sizeof(TPayload)));
    }
};
