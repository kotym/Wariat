#pragma once
#include "ComInterface.hpp"
#include "driver/uart.h"


class UartInterface : public WariatCommon::ComInterface<UartInterface>
{
    template<class T>
    void SendPacket(WariatCommon::Packet<T> packet){
        uart_write_bytes(UART_NUM_0, &packet, sizeof(packet));
    }
};