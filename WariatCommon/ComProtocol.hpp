#pragma once
#include <stdint.h>

namespace WariatCommon
{

enum class PacketPayloadType
{
    None = 0,
    HcSr04Reading,
    Stop,
    OdometryReading,
    MoveForward,
    Rotate

};

inline uint8_t CalcCheckSum(PacketPayloadType payloadType, uint8_t* data, uint8_t size)
{
    uint8_t* dataEnd = data + size;
    uint8_t checkSum = (uint8_t)payloadType;
    for (; data < dataEnd; ++data)
    {
        checkSum ^= *data;
    }
    return checkSum;
}

template <class T>
struct Packet
{
    Packet(T& _payload) : payload(_payload), payloadType(payload.GetPayloadType()) {}
    Packet(PacketPayloadType _payloadType, T& _payload) : payloadType(_payloadType), payload(_payload) {}
    void CalculateCheckSum()
    {
        checkSum = CalcCheckSum(payloadType, &payload, sizeof(payload));
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
    
    struct HcSr04Reading : public Payload
    {
        HcSr04Reading(int8_t _id, int16_t _distance) : id(_id), distance(_distance) {}
        PacketPayloadType GetPayloadType() { return PacketPayloadType::HcSr04Reading; }

        int8_t id = -1;
        int16_t distance = -1;
    };
    
    struct MoveForward : public Payload
    {
        MoveForward(int16_t _distanceCm) : distanceCm(_distanceCm) {}
        PacketPayloadType GetPayloadType() { return PacketPayloadType::MoveForward; }

        int16_t distanceCm = 0; //cm
    };
    
    struct Rotate : public Payload
    {
        Rotate(int16_t _angle) : angle(_angle) {}
        PacketPayloadType GetPayloadType() { return PacketPayloadType::Rotate; }

        int16_t angle = 0; //rad
    };
}


inline uint8_t GetPayloadSize(PacketPayloadType type)
{
    switch (type)
    {
    case PacketPayloadType::None:
    default:        
        return 0xFF;
    case PacketPayloadType::HcSr04Reading:
        return sizeof(Payload::HcSr04Reading);
    }
}
}

/*
namespace Message
{

enum class MessageType : uint8_t
{
    None,
    Message,
    Status,
    HCSR04Reading,
    Drive,
    Turn,
    Stop,
};

struct Message
{
    char txt[250];
};

struct Status
{
    int32_t messagesSent;
    int32_t messagesReceived;
    
};

struct HCSR04Reading
{
    int8_t sensorId;
    int32_t distanceToObject;
};

struct Drive
{
    int32_t distance;
};

struct Turn
{
    int32_t angle;
};

struct Stop
{
    
};

}

*/