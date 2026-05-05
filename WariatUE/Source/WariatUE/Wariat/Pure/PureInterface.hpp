#pragma once
#include "../../WariatCommon/ComInterface.hpp"

class PureInterface : WariatCommon::ComInterface<PureInterface>
{
    template <class T>
    void SendPacket(WariatCommon::Packet<T> packet)
    {
           // TODO
    }
};