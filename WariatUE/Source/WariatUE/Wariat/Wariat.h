// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "WheeledVehiclePawn.h"
#include "Pure/PureWariat.h"
#include "Pure/PureMap.h"
#include "Wariat.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UChaosWheeledVehicleMovementComponent;
class UInputAction;
struct FInputActionValue;

class UHC_SR04;


UCLASS()
class WARIATUE_API AWariat : public AWheeledVehiclePawn
{
	GENERATED_BODY()


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

	PureWariat pureWariat;
	PureMap pureMap;

	FVector2D ZeroLocation;
	float ZeroRotation;

	TArray<FVector2D> HC_SR04OffsetLocation;
	TArray<float> HC_SR04OffsetRotation;

public:
	// Sets default values for this pawn's properties
	AWariat();
	PureMap& GetPureMap() { return pureMap; }
	FVector2D GetZeroLocation() const { return ZeroLocation; }
	//const FVector2D* GetHC_SR04Offset() const { return HC_SR04OffsetLocation; }
	void GetHC_SR04RelativePos(FVector2D& Pos, float& Rotation, int Id);

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
