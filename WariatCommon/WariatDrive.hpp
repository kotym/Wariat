#pragma once
#include "ComMath.hpp"
#include "ComProtocol.hpp"

namespace WariatCommon
{

	enum class EDriveState
	{
		None,
		DriveTo,
		LookAt,
		Rotate,
		MoveForward,
		RotateAndDrive
	};

template<typename ComInterfaceClass>
class WariatDrive
{
public:
	typedef void(*DriveCallback)(void);

	//WariatDrive(ComInterfaceClass& comInterfaceRef)
	//	: comInterface(&comInterfaceRef)
	//{}

	WariatDrive()
		: comInterface(nullptr)
	{}

	void BindComInterface(ComInterfaceClass& comInterfaceRef)
	{
		comInterface = &comInterfaceRef;
	}

	bool IsBound() const
	{
		return comInterface != nullptr;
	}

	EDriveState driveState = EDriveState::None;
	Transform transform;
	Vector2<float> target;
	float distToDrive = 0;

	bool bDriving = false;
	DriveCallback driveCallback = nullptr;

	float lastRotation = 0;
	float lastMove = 0;

	bool DriveTo(const Vector2<float>& destination, DriveCallback callback = nullptr)
	{
		if (bDriving) 
			return false;
		driveState = EDriveState::DriveTo;
		bDriving = true;
		target = destination;
		driveCallback = callback;
		DriveToInternal(destination);
		return true;
	}

	bool LookAt(const Vector2<float>& destination, DriveCallback callback = nullptr)
	{
		if (bDriving) 
			return false;
		driveState = EDriveState::LookAt;
		bDriving = true;
		target = destination;
		driveCallback = callback;
		LookAtInternal(destination);
		return true;
	}

	bool Rotate(float angleRad, DriveCallback callback = nullptr)
	{
		if (bDriving) 
			return false;
		driveState = EDriveState::Rotate;
		bDriving = true;
		driveCallback = callback;
		RotateInternal(angleRad);
		return true;
	}

	bool MoveForward(float distanceCm, DriveCallback callback = nullptr)
	{
		if (bDriving) 
			return false;
		driveState = EDriveState::MoveForward;
		bDriving = true;
		driveCallback = callback;
		MoveForwardInternal(distanceCm);
		return true;
	}

	bool RotateAndDrive(float angleRad, float distanceCm, DriveCallback callback = nullptr)
	{
		if (bDriving) 
			return false;
		driveState = EDriveState::RotateAndDrive;
		bDriving = true;
		driveCallback = callback;
		distToDrive = distanceCm;
		RotateInternal(angleRad);
		return true;
	}

	void MoveFinished()
	{
		// Update position
		Vector2<float> forwardVector(cosf(transform.rotation), sinf(transform.rotation));
		//transform.position += forwardVector * lastMove;
		lastMove = 0;
		bDriving = false;
		driveState = EDriveState::None;
		if (driveCallback) driveCallback();
	}

	void RotationFinished()
	{
		// Update rotation
		//transform.rotation = NormalizeAngle(transform.rotation + lastRotation);
		lastRotation = 0;

		switch (driveState)
		{
			case EDriveState::DriveTo:
				driveState = EDriveState::MoveForward;
				MoveForwardInternal((target - transform.position).Length());
				break;
			case EDriveState::RotateAndDrive:
				driveState = EDriveState::MoveForward;
				MoveForwardInternal(distToDrive);
				break;
			default:
				bDriving = false;
				driveState = EDriveState::None;
				if (driveCallback) driveCallback();
				break;
		}
	}

	void Stop()
	{
		if (!comInterface)
			return;
		comInterface->SendData(WariatCommon::Payload::Stop());
		driveState = EDriveState::None;
		target = 0;
		bDriving = false;
		driveCallback = nullptr;
		// TODO In order to don't get lost update position from odometry or reset everything on stop
		lastRotation = 0;
		lastMove = 0;
	}

	void ResetDrive()
	{
		driveState = EDriveState::None;
		transform = Transform();
		target = 0;
		bDriving = false;
		driveCallback = nullptr;
		lastRotation = 0;
		lastMove = 0;
	}

	void OdometryUpdate(const Payload::OdometryReading& odometryReading)
	{
		transform.rotation = NormalizeAngle(transform.rotation + odometryReading.rotatedBy);
		transform.position.x += cosf(transform.rotation) * odometryReading.movedBy;
		transform.position.y += sinf(transform.rotation) * odometryReading.movedBy;
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
		RotateInternal(NormalizeAngle(angle - transform.rotation));
	}

	void RotateInternal(float angleRad)
	{
		if (!comInterface)
			return;
		if (angleRad == 0)
		{
			RotationFinished();
			return;
		}
		lastRotation = angleRad;
		comInterface->SendData(WariatCommon::Payload::Rotate(angleRad));
	}

	void MoveForwardInternal(float distanceCm)
	{
		if (!comInterface)
			return;		
		if (distanceCm == 0)
		{
			MoveFinished();
			return;
		}
		lastMove = distanceCm;
		comInterface->SendData(WariatCommon::Payload::MoveForward(distanceCm));
	}

	ComInterfaceClass* comInterface;
};

}