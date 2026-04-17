#pragma once

class Motors
{
    Motors();
    ~Motors();

public:
    void MoveForeward(float distanceCm);
    void Rotate(float angleRad);

private:
    float encoderClicksPerFullRotation = 720;
    float wheelDiameterCm = 5.6; //cm
    float wheelCircumferenceCm = 17.6; //cm
    float wheelbaseCm = 10;
};