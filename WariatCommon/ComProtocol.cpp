// #include "ComProtocol.hpp"

// namespace WariatCommon
// {

// uint8_t CalcCheckSum(PacketPayloadType payloadType, uint8_t* data, uint8_t size)
// {
//     uint8_t* dataEnd = data + size;
//     uint8_t checkSum = (uint8_t)payloadType;
//     for (; data < dataEnd; ++data)
//     {
//         checkSum ^= *data;
//     }
//     return checkSum;
// }

// uint8_t GetPayloadSize(PacketPayloadType type)
// {
//     switch (type)
//     {
//     case PacketPayloadType::None:
//     default:        
//         return 0xFF;
//     case PacketPayloadType::HcSr04Reading:
//         return sizeof(Payload::HcSr04Reading);
//     }
// }

// }