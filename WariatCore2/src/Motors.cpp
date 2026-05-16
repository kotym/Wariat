#include "Motors.hpp"
#include "hFramework.h"
#include "COREInterface.hpp"

Motors::Motors()
{
    hMot1.setEncoderPolarity(Polarity::Reversed);
    hMot1.setMotorPolarity(Polarity::Normal);
    hMot2.setEncoderPolarity(Polarity::Reversed);
    hMot2.setMotorPolarity(Polarity::Normal);
    
    Serial.printf("Motor init");
    //odometry reading
    sys.taskCreate([&](){
        while (true)
        {
            //Serial.printf("Odo: read");
            ReadOdometry();
            sys.delay(100);
        }
    });

    // TODO remove debug
    sys.taskCreate([&](){
        while (true)
        {
            if (Serial.waitForData(INFINITE))
            {
                char c = Serial.getch();
                Serial.flushRx();
                static float speed = 4, rotSpeed = M_PI / 6;
                
                switch (c)
                {
                case 'w':
                    MoveForward(speed);
                    break;
                case 's':
                    MoveForward(-speed);
                    break;
                case 'a':
                    Rotate(rotSpeed);
                    break;
                case 'd':
                    Rotate(-rotSpeed);
                    break;
                case 'm':
                    rotSpeed += M_PI / 6;
                    Serial.printf("Rot speed: %f", rotSpeed);
                    break;
                case 'n':
                    rotSpeed -= M_PI / 6;
                    Serial.printf("Rot speed: %f", rotSpeed);
                    break;
                case 'j':
                    speed -= 2;
                    Serial.printf("speed: %f", speed);
                    break;
                case 'k':
                    speed += 2;
                    Serial.printf("speed: %f", speed);
                    break;
                case 'c':
                    hMot1.rotRel(encoderClicksPerFullRotation, 300);
                    hMot2.rotRel(encoderClicksPerFullRotation, 300);
                    Serial.printf("speed: %f", speed);
                    break;
                case 'x':
                    ++encoderClicksPerFullRotation;
                    Serial.printf("encoderClicksPerFullRotation: %f", encoderClicksPerFullRotation);
                    break;
                case 'z':
                    --encoderClicksPerFullRotation;
                    Serial.printf("encoderClicksPerFullRotation: %f", encoderClicksPerFullRotation);
                    break;
                default:
                    break;
                }
            }
        }
    });
}

Motors::~Motors()
{

}

void Motors::MoveForward(float distanceCm)
{
    if (state != EState::None) return;

    const float rotations = distanceCm / wheelCircumferenceCm;
    const int32_t encoderClicks = -rotations * encoderClicksPerFullRotation; // withuout minus it drives backwards
    hMot1.rotRel(encoderClicks, power);
    hMot2.rotRel(encoderClicks, power);
    state = EState::Moving;
    sys.taskCreate([&](){
        hMot1.waitDone(); 
        hMot2.waitDone(); 
        ReadOdometry();
        state = EState::None;
        COREInterface::Get().SendEvent(WariatCommon::Payload::MoveFinished());
        Serial.printf("Mov finished");
    });
}

void Motors::Rotate(float angleRad)
{
    if (state != EState::None) return;

    const float wheelDistanceToTravel = angleRad * M_1_PI;
    const int32_t encoderClicks = wheelDistanceToTravel * encoderClicksPerFullRotation;
    hMot1.rotRel(-encoderClicks, power);
    hMot2.rotRel(encoderClicks, power);
    state = EState::Rotating;
        sys.taskCreate([&](){
        hMot1.waitDone(); 
        hMot2.waitDone(); 
        ReadOdometry();
        state = EState::None;
        COREInterface::Get().SendEvent(WariatCommon::Payload::RotationFinished());
        Serial.printf("Rot finished");
    });
}

void Motors::Stop()
{
    hMot1.stop();
    hMot2.stop();
    ReadOdometry();
    state = EState::None;
}

void Motors::ReadOdometry()
{
    static int32_t encoder1 = 0, encoder2 = 0;
    switch (state)
    {
    case EState::Moving:
    {
        const int32_t encoderCnt1 = hMot1.getEncoderCnt();
        const int32_t encoderCnt2 = hMot2.getEncoderCnt();
        float dist = ((encoderCnt1 - encoder1) + (encoderCnt2 - encoder2)) * 0.5f / encoderClicksPerFullRotation * wheelCircumferenceCm;
        COREInterface::Get().SendEvent(WariatCommon::Payload::OdometryReading(dist, 0));
        Serial.printf("Odo: dist: %f\n", dist);

        encoder1 = encoderCnt1;
        encoder1 = encoderCnt2;
    }
        break;
    case EState::Rotating:
    {
        const int32_t encoderCnt1 = hMot1.getEncoderCnt();
        const int32_t encoderCnt2 = hMot2.getEncoderCnt();
        float angle = (encoderCnt2 - encoder2 - (encoderCnt1 - encoder1)) * 0.5f / encoderClicksPerFullRotation * M_TWOPI;
        COREInterface::Get().SendEvent(WariatCommon::Payload::OdometryReading(0, angle));
        Serial.printf("Odo: rot: %f\n", angle);
        encoder1 = encoderCnt1;
        encoder1 = encoderCnt2;
    }
        break;
    case EState::None:
    default:
        break;
    }
}
