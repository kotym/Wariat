// Copyright Epic Games, Inc. All Rights Reserved.

#include "WariatUEWheelFront.h"
#include "UObject/ConstructorHelpers.h"

UWariatUEWheelFront::UWariatUEWheelFront()
{
	AxleType = EAxleType::Front;
	bAffectedBySteering = true;
	MaxSteerAngle = 40.f;
}