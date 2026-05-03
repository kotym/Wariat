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