#pragma once
#include <stdint.h>

namespace WariatCommon
{

enum class PacketPayloadType : uint8_t
{
    None = 0,
    Stop,
    HcSr04Reading,
    OdometryReading,
    MoveForward,
    Rotate,
    RotateAndMove, // TODO remove
    BlinkToggle,
    MoveFinished,
    RotationFinished
};

inline uint8_t CalcCheckSum(PacketPayloadType payloadType, uint8_t* data, uint8_t size)
{
    const uint8_t* dataEnd = data + size;
    uint8_t checkSum = (uint8_t)payloadType;
    for (; data < dataEnd; ++data)
    {
        checkSum ^= *data;
    }
    return checkSum;
}

#pragma pack(push, 1)

template <class T>
struct Packet
{
    Packet(T& _payload) : payloadType(_payload.GetPayloadType()), payload(_payload) { CalculateCheckSum(); }
    Packet(PacketPayloadType _payloadType, T& _payload) : payloadType(_payloadType), payload(_payload) { CalculateCheckSum(); }
    void CalculateCheckSum()
    {
        checkSum = CalcCheckSum(payloadType, (uint8_t*)&payload, sizeof(payload));
    }

    const uint8_t startValue = 0xAA;
    PacketPayloadType payloadType = PacketPayloadType::None;
    T payload; // TODO should be & ???
    uint8_t checkSum = 0;
    const uint8_t endValue = 0xFA;
};

namespace Payload
{
    struct Payload
    {
        PacketPayloadType GetPayloadType() { return PacketPayloadType::None; }
    };
    
    struct Stop : public Payload
    {
        PacketPayloadType GetPayloadType() { return PacketPayloadType::Stop; }
    };
    
    struct HcSr04Reading : public Payload
    {
        HcSr04Reading(int8_t _id, int16_t _distance) : id(_id), distance(_distance) {}
        PacketPayloadType GetPayloadType() { return PacketPayloadType::HcSr04Reading; }

        int8_t id = -1;
        int16_t distance = -1;
    };
    
    struct OdometryReading : public Payload
    {
        OdometryReading(float move, float rotation) : movedBy(move), rotatedBy(rotation) {}
        PacketPayloadType GetPayloadType() { return PacketPayloadType::OdometryReading; }

        float movedBy = 0.f;
        float rotatedBy = 0.f;
    };
    
    struct MoveForward : public Payload
    {
        MoveForward(float _distanceCm) : distanceCm(_distanceCm) {}
        PacketPayloadType GetPayloadType() { return PacketPayloadType::MoveForward; }

        float distanceCm = 0; //cm
    };
    
    struct Rotate : public Payload
    {
        Rotate(float _angle) : angle(_angle) {}
        PacketPayloadType GetPayloadType() { return PacketPayloadType::Rotate; }

        float angle = 0; //rad
    };
    
    struct RotateAndMove : public Payload
    {
        RotateAndMove(float _angle, float _distanceCm) : angle(_angle), distanceCm(_distanceCm) {}
        PacketPayloadType GetPayloadType() { return PacketPayloadType::RotateAndMove; }

        float angle = 0; //rad
        float distanceCm = 0; //rad
    };
    
    struct BlinkToggle : public Payload
    {
        PacketPayloadType GetPayloadType() { return PacketPayloadType::BlinkToggle; }

    };
    
    struct MoveFinished : public Payload
    {
        PacketPayloadType GetPayloadType() { return PacketPayloadType::MoveFinished; }
    };
    
    struct RotationFinished : public Payload
    {
        PacketPayloadType GetPayloadType() { return PacketPayloadType::RotationFinished; }
    };
#pragma pack(pop)
}

inline uint8_t GetPayloadSize(PacketPayloadType type)
{
    switch (type)
    {
    case PacketPayloadType::None:
    default:        
        return 0;
    case PacketPayloadType::Stop:
        return sizeof(Payload::Stop);
    case PacketPayloadType::HcSr04Reading:
        return sizeof(Payload::HcSr04Reading);
    case PacketPayloadType::OdometryReading:
        return sizeof(Payload::OdometryReading);
    case PacketPayloadType::MoveForward:
        return sizeof(Payload::MoveForward);
    case PacketPayloadType::Rotate:
        return sizeof(Payload::Rotate);
    case PacketPayloadType::RotateAndMove:
        return sizeof(Payload::RotateAndMove);
    case PacketPayloadType::BlinkToggle:
        return sizeof(Payload::BlinkToggle);
    case PacketPayloadType::MoveFinished:
        return sizeof(Payload::MoveFinished);
    case PacketPayloadType::RotationFinished:
        return sizeof(Payload::RotationFinished);
    }
}

}