// Fill out your copyright notice in the Description page of Project Settings.
#include "Wariat/Wariat.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "WariatUE.h"
#include "HC_SR04.h"
#include "../WariatCommon/ComMath.hpp"

// Sets default values
AWariat::AWariat()
	: pureWariat(this)
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(GetMesh());
	BackSpringArm->TargetArmLength = 650.0f;
	BackSpringArm->SocketOffset.Z = 150.0f;
	BackSpringArm->bDoCollisionTest = true;
	BackSpringArm->bInheritPitch = true;
	BackSpringArm->bInheritYaw = true;
	BackSpringArm->bInheritRoll = true;
	BackSpringArm->bUsePawnControlRotation = true;
	BackSpringArm->bEnableCameraRotationLag = true;
	BackSpringArm->CameraRotationLagSpeed = 5.0f;
	BackSpringArm->CameraLagMaxDistance = 50.0f;

	BackCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("Back Camera"));
	BackCamera->SetupAttachment(BackSpringArm);

	GetMesh()->SetSimulatePhysics(true);
	GetMesh()->SetCollisionProfileName(FName("Vehicle"));
	
	ChaosVehicleMovement = CastChecked<UChaosWheeledVehicleMovementComponent>(GetVehicleMovement());
}

// Called when the game starts or when spawned
void AWariat::BeginPlay()
{
	Super::BeginPlay();

	GetComponents<UHC_SR04>(HC_SR04s);

	ResetWariat();
}

void AWariat::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//FMemory::Free(map);
}

// Called every frame
void AWariat::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	CheckHC_SR04();

}

// Called to bind functionality to input
void AWariat::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(MoveIA, ETriggerEvent::Triggered, this, &AWariat::Move);
		EnhancedInputComponent->BindAction(MoveIA, ETriggerEvent::Completed, this, &AWariat::Move);

		// look around
		EnhancedInputComponent->BindAction(LookAroundIA, ETriggerEvent::Triggered, this, &AWariat::LookAround);
		EnhancedInputComponent->BindAction(ResetIA, ETriggerEvent::Completed, this, &AWariat::ResetWariat);
	}
	else
	{
		UE_LOG(LogWariatUE, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AWariat::Move(const FInputActionValue& Value)
{
	const FVector2D& Input = Value.Get<FInputActionValue::Axis2D>();
	if (Input.Y >= 0) {
		ChaosVehicleMovement->SetThrottleInput(Input.Y);
		ChaosVehicleMovement->SetBrakeInput(0);
	}
	else {
		ChaosVehicleMovement->SetThrottleInput(0);
		ChaosVehicleMovement->SetBrakeInput(-Input.Y);
	}

	ChaosVehicleMovement->SetSteeringInput(Input.X);
}

void AWariat::LookAround(const FInputActionValue& Value)
{
	const FVector2D& Input = Value.Get<FVector2D>();

	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void AWariat::GetHC_SR04RelativePos(FVector2D& Pos, float& Rotation, int Id)
{
	const float WariatRot = GetActorRotation().Yaw - ZeroRotation + 360.f;
	const float WariatRotRad = FMath::DegreesToRadians(FMath::Modulo(WariatRot, 360.f));
	Rotation = FMath::DegreesToRadians(FMath::Modulo(WariatRot + HC_SR04OffsetRotation[Id] + 360.f, 360.f));
	Pos = FVector2D(GetActorLocation()) - ZeroLocation;
	const float Cos = cosf(WariatRotRad), Sin = sinf(WariatRotRad);
	Pos.X += HC_SR04OffsetLocation[Id].X * Cos - HC_SR04OffsetLocation[Id].Y * Sin;
	Pos.Y += HC_SR04OffsetLocation[Id].X * Sin + HC_SR04OffsetLocation[Id].Y * Cos;
}

void AWariat::ResetWariat()
{
	pureMap.Reset();
	ZeroLocation = FVector2D(GetActorLocation());
	ZeroRotation = 0;// GetActorRotation().Yaw;
	HC_SR04OffsetLocation.Empty(HC_SR04s.Num());
	HC_SR04OffsetRotation.Empty(HC_SR04s.Num());
	for (UHC_SR04* HC_SR04 : HC_SR04s)
	{
		if (HC_SR04 == nullptr) continue;
		HC_SR04OffsetLocation.Add(FVector2D(HC_SR04->GetRelativeLocation()));
		HC_SR04OffsetRotation.Add(HC_SR04->GetRelativeRotation().Yaw);
	}
}

void AWariat::CheckHC_SR04()
{
	pureMap.ResetOutline();
	
	FVector2D posCenter(FVector2D(GetActorLocation()) - ZeroLocation);
	posCenter /= pureMap.GetCellSizeInCm();
	Vector2<int32_t> posCenerInt(posCenter.X, posCenter.Y);
	int i = 0;
	for (UHC_SR04* HC_SR04 : HC_SR04s)
	{
		if (HC_SR04 == nullptr) continue;
		HC_SR04->SphereConeTrace();
		float Dist = HC_SR04->GetLastDetectionDist();
		float Rotation;
		
		FVector2D pos;
		GetHC_SR04RelativePos(pos, Rotation, i++);
		pos /= pureMap.GetCellSizeInCm();
		Dist /= pureMap.GetCellSizeInCm();
		Vector2<int32_t> MapVector(pos.X, pos.Y);

		pureMap.UpdateMapFromScan(MapVector, Rotation, Dist - 1, HC_SR04->GetDetectionAngleRad(), HC_SR04->WasLastDetectionHit(), MapVector - posCenerInt);
	
		FVector p = HC_SR04->GetComponentLocation();
		FVector s1(pos.X * pureMap.GetCellSizeInCm() + ZeroLocation.X, pos.Y * pureMap.GetCellSizeInCm() + ZeroLocation.Y, p.Z);
		DrawDebugDirectionalArrow(GetWorld(), s1, s1 + FRotator(0, FMath::RadiansToDegrees(Rotation), 0).Vector() * 50, 10, FColor::Magenta, false, 0, 250, 1);
	}
}

