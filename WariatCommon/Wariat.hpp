#pragma once
#include <concepts>
#include <inttypes.h>
#include "ComMap.hpp"
//#include "EventDispatcher.hpp"
#include "ComNavi.hpp"
#include "ComProtocol.hpp"
#include "ComInterface.hpp"
#include "MapRenderer.hpp"

namespace WariatCommon
{

enum class EWariatState
{
	None,
	Manual,
	Autonomous
};

template<class T>
concept MapRendererDerived = std::derived_from<T, MapRenderer<T>>;

template<class T>
concept ComInterfaceDerived = std::derived_from<T, ComInterface<T>>;

template<std::derived_from<ComMap> MapClass, MapRendererDerived MapRendererClass, ComInterfaceDerived ComInterfaceClass>
class Wariat
{
public:
	static Wariat& Get() {
		static Wariat wariat;
		return wariat;
	}

	Wariat() : map(), comInterface(), navi(map, comInterface), mapRenderer() {}

	MapClass map;
	ComInterfaceClass comInterface;
	ComNavi<ComInterfaceClass> navi;
	MapRendererClass mapRenderer;
	//EventDispatcher eventDispatcher;

	//int32_t hcSr04Readings[4];
	Transform hcSr04Offsets[4];
	//Wheels position ?

	Transform transform;
	// current hsm state

	EWariatState state = EWariatState::Autonomous;

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
	
    template<std::derived_from<WariatCommon::Payload::Payload> PayloadClass>
    void SendData(PayloadClass payload)
    {
        comInterface.SendData(payload);
    }

	void SetState(EWariatState newState)
	{
		state = newState;
		navi.Reset();
	}

	void Update()
	{
		mapRenderer.RenderMap(transform, map);

		if (state == EWariatState::Autonomous)
			navi.Update(transform);

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
		printf("hcSr04ReadingProcessing: id: %d, distance: %d", (int)hcSr04Reading.id, hcSr04Reading.distance);
		if (hcSr04Reading.id == 0) 
			map.ResetOutline();
		Transform sensorTransform;
		GetHcSr04RelativePos(sensorTransform, hcSr04Reading.id);
		map.UpdateMapFromScan(sensorTransform, hcSr04Reading.distance * 0.9f, 0.5235987755f /* 30 deg in rad */, hcSr04Reading.distance < 300, sensorTransform.position - transform.position);
	}

};

}
