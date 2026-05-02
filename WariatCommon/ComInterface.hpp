#pragma once
#include <concepts>
#include "ComProtocol.hpp"

namespace WariatCommon
{

//class ComInterface;
//template <std::derived_from<ComInterface> Derived>
template<class Derived>
class ComInterface
{
    template <class T>
    void SendPacket(Packet<T> packet)
    {
        Derived::SendPacket(packet);
    }
};

}