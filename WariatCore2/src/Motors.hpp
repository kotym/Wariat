#pragma once
#include "ComProtocol.hpp"
#include "ComMath.hpp"

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
    //float wheelDiameterCm = 5.6; //cm
    float wheelCircumferenceCm = 8.3f * M_PI; //cm
    //float wheelbaseCm = 13.3f;
    int16_t power = 500;
    static Motors* motors;
};

inline Motors* Motors::motors = nullptr;