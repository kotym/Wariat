#pragma once
#include <concepts>
#include <inttypes.h>
#include "ComMap.hpp"
//#include "EventDispatcher.hpp"
#include "ComNavi.hpp"
#include "ComProtocol.hpp"
#include "ComInterface.hpp"
#include "MapRenderer.hpp"
#include "WariatDrive.hpp"

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
class Wariat : public WariatDrive<ComInterfaceClass>
{
public:
	Wariat() : map(), comInterface(), navi(map, *this), mapRenderer()
	{
		this->BindComInterface(comInterface);
	}

	MapClass map;
	ComInterfaceClass comInterface;
	ComNavi<ComInterfaceClass> navi;
	MapRendererClass mapRenderer;
	//EventDispatcher eventDispatcher;

	//int32_t hcSr04Readings[4];
	Transform hcSr04Offsets[4];
	//Wheels position ?


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
			{
				Payload::OdometryReading& odometryReading = *static_cast<Payload::OdometryReading*>(payload);
				this->OdometryUpdate(odometryReading);
				break;
			}
			case PacketPayloadType::MoveFinished:
				this->MoveFinished();
				break;
			case PacketPayloadType::RotationFinished:
				this->RotationFinished();
				break;
		}
	}
	
    template<std::derived_from<WariatCommon::Payload::Payload> PayloadClass>
    void SendData(PayloadClass payload)
    {
        comInterface.SendData(payload);
    }

	void Reset()
	{
		state = EWariatState::Autonomous;
		this->ResetDrive();
		navi.Reset();
	}

	void SetState(EWariatState newState)
	{
		state = newState;
		navi.Reset();
	}

	void Update()
	{
		mapRenderer.RenderMap(this->transform, map);

		if (state == EWariatState::Autonomous)
			navi.Update(this->transform);

	}

	void GetHcSr04RelativePos(Transform& outTransform, int8_t id)
	{
		outTransform = this->transform;
		outTransform.rotation += hcSr04Offsets[id].rotation;
		const float cos = cosf(this->transform.rotation), sin = sinf(this->transform.rotation);
		outTransform.position.x += hcSr04Offsets[id].position.x * cos - hcSr04Offsets[id].position.y * sin;
		outTransform.position.y += hcSr04Offsets[id].position.x * sin + hcSr04Offsets[id].position.y * cos;
	}

	void ProcessHcSr04Reading(Payload::HcSr04Reading& hcSr04Reading)
	{
		//printf("hcSr04ReadingProcessing: id: %d, distance: %d\n", (int)hcSr04Reading.id, hcSr04Reading.distance);
		if (hcSr04Reading.id == 0) 
			map.ResetOutline();
		Transform sensorTransform;
		GetHcSr04RelativePos(sensorTransform, hcSr04Reading.id);
		map.UpdateMapFromScan(sensorTransform, hcSr04Reading.distance * 0.9f, 0.5235987755f /* 30 deg in rad */, hcSr04Reading.distance < 300, sensorTransform.position - this->transform.position);
	}

};

}
