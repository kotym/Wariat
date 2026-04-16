#pragma once

#include <cstdint>

enum class MindStateId : uint8_t
{
    None = 0,
    Idle = 1,
    MoveForward = 2,
};
