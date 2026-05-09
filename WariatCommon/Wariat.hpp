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

class WariatDriveInterface
{
public:
	typedef void(*DriveCallback)(void);

	enum class EDriveState
	{
		None,
		DriveTo,
		LookAt,
		Rotate,
		MoveForward
	} state = EDriveState::None;

	Transform transform;
	Vector2<float> target;

	bool bDriving = false;
	DriveCallback driveCallback = nullptr;

	float lastRotation = 0;
	float lastMove = 0;

	bool DriveTo(const Vector2<float>& destination, DriveCallback callback = nullptr)
	{
		if (bDriving) return false;
		state = EDriveState::DriveTo;
		bDriving = true;
		target = destination;
		driveCallback = callback;
		DriveToInternal(destination);
		return true;
	}

	bool LookAt(const Vector2<float>& destination, DriveCallback callback = nullptr)
	{
		if (bDriving) return false;
		state = EDriveState::LookAt;
		bDriving = true;
		target = destination;
		driveCallback = callback;
		LookAtInternal(destination);
		return true;
	}

	bool Rotate(float angleRad, DriveCallback callback = nullptr)
	{
		if (bDriving) return false;
		state = EDriveState::Rotate;
		bDriving = true;
		driveCallback = callback;
		RotateInternal(angleRad);
		return true;
	}

	bool MoveForward(float distanceCm, DriveCallback callback = nullptr)
	{
		if (bDriving) return false;
		state = EDriveState::MoveForward;
		bDriving = true;
		driveCallback = callback;
		MoveForwardInternal(distanceCm);
		return true;
	}

	void MoveFinished()
	{
		// Update position
		Vector2<float> forwardVector(cosf(transform.rotation), sinf(transform.rotation));
		transform.position += forwardVector * lastMove;
		lastMove = 0;
		if (driveCallback) driveCallback();
	}

	void RotationFinished()
	{
		// Update rotation
		transform.rotation = NormalizeAngle(transform.rotation + lastRotation);

		switch (state)
		{
			case WariatCommon::WariatDriveInterface::EDriveState::DriveTo:
				MoveForwardInternal((target - transform.position).Length());
				break;
			default:
				if (driveCallback) driveCallback();
				break;
		}
	}

	void Stop()
	{
		comInterface.SendData(WariatCommon::Payload::Stop());
		state = EDriveState::None;
		target = 0;
		bDriving = false;
		driveCallback = nullptr;
		// TODO In order to don't get lost update position from odometry or reset everything on stop
		lastRotation = 0;
		lastMove = 0;
	}

	void ResetDrive()
	{
		state = EDriveState::None;
		transform = Transform();
		target = 0;
		bDriving = false;
		driveCallback = nullptr;
		lastRotation = 0;
		lastMove = 0;
	}

private:
	void DriveToInternal(const Vector2<float>& destination)
	{
		LookAtInternal(destination);
	}

	void LookAtInternal(const Vector2<float>& destination)
	{
		const Vector2<float> path = destination - transform.position;
		const float angle = atan2f(path.x, path.y);
		RotateInternal(angle);
	}

	void RotateInternal(float angleRad)
	{
		lastRotation = angleRad;
		comInterface.SendData(WariatCommon::Payload::Rotate(NormalizeAngle(angleRad)));
	}

	void MoveForwardInternal(float distanceCm)
	{
		lastMove = distanceCm;
		comInterface.SendData(WariatCommon::Payload::MoveForward(distanceCm));
	}
};


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
class Wariat : public WariatDriveInterface
{
public:
	Wariat() : map(), comInterface(), navi(map, comInterface), mapRenderer() {}

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
				// TODO
				// Update odometry position
				break;
			case PacketPayloadType::MoveFinished:
				MoveFinished();
				break;
			case PacketPayloadType::RotationFinished:
				RotationFinished();
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
		ResetDrive();
		navi.Reset();
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
