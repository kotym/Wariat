#include "PureInterface.hpp"
#include "../UEWariat.h"

void PureInterface::ProcessCommand(WariatCommon::PacketPayloadType payloadType, void* payload)
{
	ueWariat->ProcessCommand(payloadType, payload);
}
