// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Pure/PureWariat.h"
#include "UEWariat.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UChaosWheeledVehicleMovementComponent;
class UInputAction;
struct FInputActionValue;

class UHC_SR04;


UCLASS()
class WARIATUE_API AUEWariat : public AWheeledVehiclePawn
{
	GENERATED_BODY()

	friend class UWariatUI;

	/** Spring Arm for the back camera */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<USpringArmComponent> BackSpringArm;

	/** Back Camera component */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Components", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UCameraComponent> BackCamera;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> MoveIA;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> LookAroundIA;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	TObjectPtr<UInputAction> ResetIA;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Input", meta = (AllowPrivateAccess = "true"))
	float LookAroundSpeed = 1.f;

	UPROPERTY(VisibleAnywhere)
	TArray<TObjectPtr<UHC_SR04>> HC_SR04s;

	TObjectPtr<UChaosWheeledVehicleMovementComponent> ChaosVehicleMovement;

	//////////////////////////////////////////////////////////

	// ESP part ////////////////////////////////////////

	PureWariat pureWariat;

	// ESP helpers /////////////////////////////////////////

	FVector2D ZeroLocation;
	float ZeroRotation;

	// CORE2 part ///////////////////////////////////////////


	// CORE2 helpers //////////////////////////////////////////


	// other Wariat helpers /////////////////////////////////////

	/////////////////////////////////////////////////////////////////////

public:
	// Sets default values for this pawn's properties
	AUEWariat();
	PureMap& GetPureMap() { return pureWariat.map; }
	FVector2D GetZeroLocation() const { return ZeroLocation; }
	//const FVector2D* GetHC_SR04Offset() const { return HC_SR04OffsetLocation; }

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;
	void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
public:	
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

protected:
	void Move(const FInputActionValue& Value);
	void LookAround(const FInputActionValue& Value);
	void ResetWariat();
	void CheckHC_SR04();
};
