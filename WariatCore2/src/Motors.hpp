#pragma once
#include "ComProtocol.hpp"

class Motors
{
    Motors();
    ~Motors();

public:
   enum class EState : uint8_t
    {
        None,
        Moving,
        Rotating,
    } state = EState::None;

    void static Init() {
        motors = new Motors();
    }
    static Motors& Get() { return *motors; }
public:
    void MoveForward(float distanceCm);
    void Rotate(float angleRad);
    void Stop();
    void ReadOdometry();

private:
    float encoderClicksPerFullRotation = 720;
    float wheelDiameterCm = 5.6; //cm
    float wheelCircumferenceCm = 17.6; //cm
    float wheelbaseCm = 10;

    static Motors* motors;
};

inline Motors* Motors::motors = nullptr;