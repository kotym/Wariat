#pragma once
#include "ComProtocol.hpp"
#include "hTypes.h"
#include "Motors.hpp"
#include <concepts>


class COREInterface
{
public:
    void static Init(hFramework::hSerial& _uart) {
        coreInterface = new COREInterface(_uart);
    }
    static COREInterface& Get() { return *coreInterface; }
private:
    COREInterface(hFramework::hSerial& _uart) : uart(_uart) 
    {
        uart.init(11520);
    }
public:
    template<std::derived_from<WariatCommon::Payload::Payload> PayloadClass>
    void SendEvent(PayloadClass payload){
        WariatCommon::Packet<PayloadClass> packet(payload);
        packet.CalculateCheckSum();
        uart.write(packet, sizeof(packet));
    }

    void ReceiveCommands()
    {
        while(true)
        {
            uint8_t state = 0;
            WariatCommon::PacketPayloadType payloadType = WariatCommon::PacketPayloadType::None;
            uint8_t payloadSize = 0;
            uint8_t payload[128];

            auto readingError = [&](){
                uart.flushRx();
                state = 0;
                // log
            };

            if (uart.waitForData(12345653))
            {
                switch (state)
                {
                case 0:
                    if (uart.available() >= 1)
                    {
                        uint8_t start = 0;
                        uart.read(&start, 1, INFINITE);
                        if(start != 0xAA)
                        {
                            readingError();
                            break;
                        }
                        state = 1;
                    }
                    break;
                case 1:
                    if (uart.available() >= 1)
                    {
                        uart.read(&payloadType, 1, INFINITE);
                        state = 2;
                    }
                    break;
                case 2:
                {
                    payloadSize = WariatCommon::GetPayloadSize(payloadType);
                    if (payloadSize == 0) {
                        state = 3;
                        break;
                    }
                    if (uart.available() >= payloadSize)
                    {
                        uart.read(payload, payloadSize, INFINITE);
                    }
                    state = 3;
                }
                    break;
                case 3:
                    if(uart.available() >= 2)
                    {
                        struct{
                            uint8_t checkSum;
                            uint8_t end;
                        } data;
                        uart.read(&data, sizeof(data), INFINITE);
                        if (data.end != 0xFA) {
                            readingError();
                            break;
                        } 
                        if (WariatCommon::CalcCheckSum(payloadType, payload, payloadSize) != data.checkSum) {
                            readingError();
                            break;
                        }
                        
                        // process payload
                        ProcessCommand(payloadType, payload);
                    }
                }
            }
        }
    }

    void ProcessCommand(WariatCommon::PacketPayloadType payloadType, void* payload)
    {
        switch (payloadType)
        {
        case WariatCommon::PacketPayloadType::Stop:
            Motors::Get().Stop();
            break;
        case WariatCommon::PacketPayloadType::MoveForward:
            Motors::Get().MoveForward(static_cast<WariatCommon::Payload::MoveForward*>(payload)->distanceCm);
            break;
        case WariatCommon::PacketPayloadType::Rotate:
            Motors::Get().Rotate(static_cast<WariatCommon::Payload::Rotate*>(payload)->angle);
            break;
        }
    }

    hFramework::hSerial& uart;

private:
    static COREInterface* coreInterface;
};

inline COREInterface* COREInterface::coreInterface = nullptr;