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
public:
    template<std::derived_from<WariatCommon::Payload::Payload> PayloadClass>
    void SendData(PayloadClass payload)
    {
        Derived::SendData(payload);
    }
};

}