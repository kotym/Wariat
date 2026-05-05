#pragma once
#include <concepts>
#include <inttypes.h>
#include "ComMap.hpp"
//#include "EventDispatcher.hpp"
#include "ComNavi.hpp"
#include "ComProtocol.hpp"
#include "ComInterface.hpp"
#include "MapRenderer.hpp"

// template<typename T, typename Base>
// concept DerivedFrom = std::is_base_of_v<Base, T>;

// template<DerivedFrom<ComMap> MapClass> ;

namespace WariatCommon
{

// class Wariat0
// {
//     ComNavi& navi;

// };


template<std::derived_from<ComMap> MapClass, class MapRendererClass, class ComInterfaceClass>
class Wariat
{
public:
    static Wariat& Get() {
        static Wariat wariat;
        return wariat;
    }

    Wariat() : navi(map) {}

    ComNavi navi;
    MapClass map;
    MapRendererClass mapRenderer;
    //EventDispatcher eventDispatcher;
    ComInterfaceClass comInterface;

    //int32_t hcSr04Readings[4];
    Transform hcSr04Offsets[4];
    //Wheels position ?

    Transform transform;
    // current hsm state

public:
    void ProcessEvent(PacketPayloadType payloadType, void* payload)
    {
        if (payload == nullptr)
        {
            return;
        }
        switch (payloadType)
        {
            case PacketPayloadType::None:
            default: 
                break;
            case PacketPayloadType::HcSr04Reading:
            {
                Payload::HcSr04Reading& hcSr04Reading = *static_cast<Payload::HcSr04Reading*>(payload);
                ProcessHcSr04Reading(hcSr04Reading);
                break;
            }
            case PacketPayloadType::Stop:
                // TODO
                // Stop everything
                break;
            case PacketPayloadType::OdometryReading:
                // TODO
                // Update odometry position
                break;
        }
    }

    template<class T>
    void SendPacket(Packet<T> packet)
    {
        comInterface.SendPacket(packet);
    }

    void Update()
    {
        mapRenderer.RenderMap(transform, map);
    }

    void GetHcSr04RelativePos(Transform& outTransform, int8_t id)
    {
        outTransform = transform;
        outTransform.rotation += hcSr04Offsets[id].rotation;
        const float cos = cosf(transform.rotation), sin = sinf(transform.rotation);
        outTransform.position.x += hcSr04Offsets[id].position.x * cos - hcSr04Offsets[id].position.y * sin;
        outTransform.position.y += hcSr04Offsets[id].position.x * sin + hcSr04Offsets[id].position.y * cos;
    }

    void ProcessHcSr04Reading(Payload::HcSr04Reading& hcSr04Reading)
    {
        if (hcSr04Reading.id == 0) 
            map.ResetOutline();
        Transform sensorTransform;
        GetHcSr04RelativePos(sensorTransform, hcSr04Reading.id);
        map.UpdateMapFromScan(sensorTransform, hcSr04Reading.distance - 1, 0.5235987755f /* 30 deg in rad */, true, sensorTransform.position - transform.position);
    }

};

}
