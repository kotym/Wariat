#pragma once
#include "ComProtocol.hpp"

class Motors
{
    Motors();
    ~Motors();

public:
    void static Init() {
        motors = new Motors();
    }
    static Motors& Get() { return *motors; }
public:
    void MoveForward(float distanceCm);
    void Rotate(float angleRad);
    void Stop();

private:
    float encoderClicksPerFullRotation = 720;
    float wheelDiameterCm = 5.6; //cm
    float wheelCircumferenceCm = 17.6; //cm
    float wheelbaseCm = 10;

    static Motors* motors;
};

inline Motors* Motors::motors = nullptr;