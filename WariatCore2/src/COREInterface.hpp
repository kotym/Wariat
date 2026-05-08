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
        uart.init(115200, Parity::None, StopBits::One);
    }
public:
    template<std::derived_from<WariatCommon::Payload::Payload> PayloadClass>
    void SendEvent(PayloadClass payload){
        WariatCommon::Packet<PayloadClass> packet(payload);
        uart.write((void*)&packet, sizeof(packet));
    }

    void ReceiveCommands()
    {
        uint8_t state = 0;
        WariatCommon::PacketPayloadType payloadType = WariatCommon::PacketPayloadType::None;
        uint8_t payloadSize = 0;
        uint8_t payload[128];

        auto readingError = [&](){
                Serial.printf("error: %d\n", state);
                uart.flushRx();
                state = 0;
                // log
            };

        while(true)
        {
            // TODO Czy to czeka blokujac??
            if (uart.waitForData(1))
            {
                Serial.printf("receiving state: %d\n", state);
                switch (state)
                {
                case 0:
                    if (uart.available() >= 1)
                    {
                        uint8_t start = 0;
                        uart.read(&start, 1, INFINITE);
                        Serial.printf("start: %d\n", start);
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
                        uart.read((void*)&payloadType, 1, INFINITE);
                        Serial.printf("payloadType: %d\n", payloadType);
                        state = 2;
                    }
                    break;
                case 2:
                {
                    payloadSize = WariatCommon::GetPayloadSize(payloadType);
                    Serial.printf("payloadSize: %d\n", payloadSize);
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
                        Serial.printf("data: %d %d\n", data.checkSum, data.end);
                        if (data.end != 0xFA) {
                            readingError();
                            break;
                        } 
                        uint8_t checkSum = WariatCommon::CalcCheckSum(payloadType, payload, payloadSize);
                        Serial.printf("checkSum: %d\n", checkSum);
                        if (checkSum != data.checkSum) {
                            readingError();
                            break;
                        }
                        
                        // process payload
                        ProcessCommand(payloadType, payload);
                    }
                }
            }
            
            sys.delay_ms(50); // TODO czy to potrzebne?
        }
    }

    void ProcessCommand(WariatCommon::PacketPayloadType payloadType, void* payload)
    {
        Serial.printf("processing command: Type: %d", (int)payloadType);
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
        case WariatCommon::PacketPayloadType::BlinkToggle:
            hLED1.toggle();
            Serial.printf("blink...");
            break;
        }
    }

    hFramework::hSerial& uart;

private:
    static COREInterface* coreInterface;
};

inline COREInterface* COREInterface::coreInterface = nullptr;