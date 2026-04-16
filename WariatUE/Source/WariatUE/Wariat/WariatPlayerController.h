// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/PlayerController.h"
#include "WariatPlayerController.generated.h"

class UInputMappingContext;
class AWariat;
class UWariatUI;

UCLASS()
class WARIATUE_API AWariatPlayerController : public APlayerController
{
	GENERATED_BODY()
	
	UPROPERTY(EditAnywhere, Category = "Input")
	TArray<UInputMappingContext*> DefaultMappingContexts;

	/** Type of vehicle to automatically respawn when it's destroyed */
	//UPROPERTY(EditAnywhere, Category = "Vehicle|Respawn")
	//TSubclassOf<AWariat> VehiclePawnClass;

	/** Pointer to the controlled vehicle pawn */
	TObjectPtr<AWariat> WariatPawn;

	/** Type of the UI to spawn */
	UPROPERTY(EditAnywhere, Category = "Vehicle|UI")
	TSubclassOf<UWariatUI> VehicleUIClass;

	/** Pointer to the UI widget */
	UPROPERTY()
	TObjectPtr<UWariatUI> VehicleUI;

protected:

	void BeginPlay() override;

	void SetupInputComponent() override;

	void OnPossess(APawn* InPawn) override;
};
