// Copyright Epic Games, Inc. All Rights Reserved.

#include "WariatUEWheelRear.h"
#include "UObject/ConstructorHelpers.h"

UWariatUEWheelRear::UWariatUEWheelRear()
{
	AxleType = EAxleType::Rear;
	bAffectedByHandbrake = true;
	bAffectedByEngine = true;
}