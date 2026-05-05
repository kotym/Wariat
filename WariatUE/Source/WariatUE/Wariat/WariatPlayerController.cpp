// Fill out your copyright notice in the Description page of Project Settings.


#include "Wariat/WariatPlayerController.h"
#include "EnhancedInputSubsystems.h"
#include "WariatUI.h"
#include "WariatUE.h"
#include "UEWariat.h"


void AWariatPlayerController::BeginPlay()
{
	Super::BeginPlay();

	// ensure we're attached to the vehicle pawn so that World Partition streaming works correctly
	bAttachToPawn = true;

	// only spawn UI on local player controllers
	if (IsLocalPlayerController())
	{
		
		// spawn the UI widget and add it to the viewport
		VehicleUI = CreateWidget<UWariatUI>(this, VehicleUIClass);

		if (VehicleUI)
		{
			VehicleUI->AddToViewport();

		}
		else {

			UE_LOG(LogWariatUE, Error, TEXT("Could not spawn vehicle UI widget."));
		}
	}
}

void AWariatPlayerController::SetupInputComponent()
{
	Super::SetupInputComponent();

	if (IsLocalPlayerController())
	{
		// Add Input Mapping Contexts
		if (UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(GetLocalPlayer()))
		{
			for (UInputMappingContext* CurrentContext : DefaultMappingContexts)
			{
				Subsystem->AddMappingContext(CurrentContext, 0);
			}
		}
	}
}

void AWariatPlayerController::OnPossess(APawn* InPawn)
{
	Super::OnPossess(InPawn);

	// get a pointer to the controlled pawn
	//WariatPawn = CastChecked<AUEWariat>(InPawn);

	// subscribe to the pawn's OnDestroyed delegate
	//VehiclePawn->OnDestroyed.AddDynamic(this, &AWariatUEPlayerController::OnPawnDestroyed);
}
