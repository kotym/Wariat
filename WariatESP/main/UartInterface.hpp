#pragma once
#include "ComInterface.hpp"
#include "driver/uart.h"


class UartInterface : public WariatCommon::ComInterface<UartInterface>
{
public:
    template<std::derived_from<WariatCommon::Payload::Payload> PayloadClass>
    void SendData(PayloadClass payload){
        WariatCommon::Packet<PayloadClass> packet(payload);
        uart_write_bytes(UART_NUM_2, &packet, sizeof(packet));
    }
};