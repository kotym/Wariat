#include "Motors.hpp"
#include "hFramework.h"

Motors::Motors()
{
    hMot1.setEncoderPolarity(Polarity::Reversed);
    hMot1.setMotorPolarity(Polarity::Normal);
    hMot2.setEncoderPolarity(Polarity::Reversed);
    hMot2.setMotorPolarity(Polarity::Normal);
    
}

Motors::~Motors()
{

}

void Motors::MoveForeward(float distanceCm)
{
    const float rotations = distanceCm / wheelCircumferenceCm;
    const int32_t encoderClicks = rotations * encoderClicksPerFullRotation;
    hMot1.rotRel(encoderClicks, 300);
    hMot2.rotRel(encoderClicks, 300);
}

void Motors::Rotate(float angleRad)
{
    const float wheelDistanceToTravel = M_PI * angleRad;
    const int32_t encoderClicks = wheelDistanceToTravel / wheelCircumferenceCm * encoderClicksPerFullRotation;
    hMot1.rotRel(-encoderClicks, 300);
    hMot2.rotRel(encoderClicks, 300);
}
