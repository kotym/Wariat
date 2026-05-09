#pragma once
#include "../../WariatCommon/ComInterface.hpp"
#include "../../WariatCommon/Wariat.hpp"


class PureInterface : public WariatCommon::ComInterface<PureInterface>
{
public:
    //typedef void(*ProcessEventFunc)(WariatCommon::PacketPayloadType, void*);
    //ProcessEventFunc processEvent = nullptr;
    class AUEWariat* ueWariat = nullptr;

    template<std::derived_from<WariatCommon::Payload::Payload> PayloadClass>
    void SendData(PayloadClass payload)
    {
        ProcessCommand(payload.GetPayloadType(), &payload);
    }

    void ProcessCommand(WariatCommon::PacketPayloadType, void*);
};