// Copyright Epic Games, Inc. All Rights Reserved.

#include "WariatUEGameMode.h"
#include "WariatUEPlayerController.h"

AWariatUEGameMode::AWariatUEGameMode()
{
	PlayerControllerClass = AWariatUEPlayerController::StaticClass();
}
