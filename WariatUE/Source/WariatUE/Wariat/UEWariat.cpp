// Fill out your copyright notice in the Description page of Project Settings.
#include "Wariat/UEWariat.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "WariatUE.h"
#include "HC_SR04.h"
#include "../WariatCommon/ComMath.hpp"

// Sets default values
AUEWariat::AUEWariat()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("Mesh"));
	RootComponent = Mesh;
	BackSpringArm = CreateDefaultSubobject<USpringArmComponent>(TEXT("Back Spring Arm"));
	BackSpringArm->SetupAttachment(Mesh);
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

}

// Called when the game starts or when spawned
void AUEWariat::BeginPlay()
{
	Super::BeginPlay();

	GetComponents<UHC_SR04>(HC_SR04s);

	ResetWariat();
}

void AUEWariat::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);

	//FMemory::Free(map);
}

// Called every frame
void AUEWariat::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
	//mind.update();
	CheckHC_SR04();
	UpdateKinematic(DeltaTime);

	FVector2D pos(FVector2D(GetActorLocation()) - ZeroLocation);
	pureWariat.transform.position = { (float)pos.X, (float)pos.Y };
	pureWariat.transform.rotation = FMath::DegreesToRadians(GetActorRotation().Yaw - ZeroRotation);
	pureWariat.Update();
}

// Called to bind functionality to input
void AUEWariat::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	if (UEnhancedInputComponent* EnhancedInputComponent = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		// steering 
		EnhancedInputComponent->BindAction(MoveIA, ETriggerEvent::Triggered, this, &AUEWariat::Move);
		EnhancedInputComponent->BindAction(MoveIA, ETriggerEvent::Completed, this, &AUEWariat::Move);

		// look around
		EnhancedInputComponent->BindAction(LookAroundIA, ETriggerEvent::Triggered, this, &AUEWariat::LookAround);
		EnhancedInputComponent->BindAction(ResetIA, ETriggerEvent::Completed, this, &AUEWariat::ResetWariat);
	}
	else
	{
		UE_LOG(LogWariatUE, Error, TEXT("'%s' Failed to find an Enhanced Input component! This template is built to use the Enhanced Input system. If you intend to use the legacy system, then you will need to update this C++ file."), *GetNameSafe(this));
	}
}

void AUEWariat::Move(const FInputActionValue& Value)
{
	const FVector2D& Input = Value.Get<FInputActionValue::Axis2D>();
	ManualInput = Input;
	bManualInputActive = !ManualInput.IsNearlyZero();
	if (bManualInputActive)
	{
		StopKinematic();
	}
}

void AUEWariat::LookAround(const FInputActionValue& Value)
{
	const FVector2D& Input = Value.Get<FVector2D>();

	AddControllerYawInput(Input.X);
	AddControllerPitchInput(Input.Y);
}

void AUEWariat::ResetWariat()
{
	pureWariat.map.Reset();
	ZeroLocation = FVector2D(GetActorLocation());
	ZeroRotation = 0;// GetActorRotation().Yaw;
	int i = 0;
	for (UHC_SR04* HC_SR04 : HC_SR04s)
	{
		if (HC_SR04 == nullptr) continue;
		FVector2D pos(HC_SR04->GetRelativeLocation());
		pureWariat.hcSr04Offsets[i].position = { (float)pos.X, (float)pos.Y };
		pureWariat.hcSr04Offsets[i].rotation = FMath::DegreesToRadians(HC_SR04->GetRelativeRotation().Yaw);
		++i;
	}
}

void AUEWariat::CheckHC_SR04()
{
	int i = 0;
	for (UHC_SR04* HC_SR04 : HC_SR04s)
	{
		if (HC_SR04 == nullptr) continue;
		HC_SR04->SphereConeTrace();
		float Dist = HC_SR04->GetLastDetectionDist();

		WariatCommon::Payload::HcSr04Reading reading(i++, Dist);
		pureWariat.ProcessEvent(WariatCommon::PacketPayloadType::HcSr04Reading, &reading);
	}
}

void AUEWariat::DriveDistance(float DistanceCm, float SpeedCmPerSec)
{
	StopKinematic();
	bDrivingDistance = !FMath::IsNearlyZero(DistanceCm);
	RemainingDistance = DistanceCm;
	CommandSpeed = FMath::Abs(SpeedCmPerSec) > 0.0f ? FMath::Abs(SpeedCmPerSec) : MaxSpeedCmPerSec;
}

void AUEWariat::DriveSpeed(float SpeedCmPerSec)
{
	StopKinematic();
	bDrivingDistance = false;
	bTurning = false;
	CommandSpeed = FMath::Clamp(SpeedCmPerSec, -MaxSpeedCmPerSec, MaxSpeedCmPerSec);
}

void AUEWariat::TurnDegrees(float Degrees, float TurnRateDegPerSec)
{
	StopKinematic();
	bTurning = !FMath::IsNearlyZero(Degrees);
	RemainingYaw = Degrees;
	CommandTurnRate = FMath::Abs(TurnRateDegPerSec) > 0.0f ? FMath::Abs(TurnRateDegPerSec) : MaxTurnRateDegPerSec;
}

void AUEWariat::StopKinematic()
{
	bDrivingDistance = false;
	bTurning = false;
	RemainingDistance = 0.0f;
	RemainingYaw = 0.0f;
	CommandSpeed = 0.0f;
	CommandTurnRate = 0.0f;
}

void AUEWariat::UpdateKinematic(float DeltaTime)
{
	float ForwardSpeed = 0.0f;
	float TurnRate = 0.0f;

	if (bManualInputActive)
	{
		ForwardSpeed = ManualInput.Y * MaxSpeedCmPerSec;
		TurnRate = ManualInput.X * MaxTurnRateDegPerSec;
	}
	else if (bDrivingDistance)
	{
		float Direction = FMath::Sign(RemainingDistance);
		float Step = CommandSpeed * DeltaTime;
		float Move = FMath::Min(FMath::Abs(RemainingDistance), Step) * Direction;
		RemainingDistance -= Move;
		ForwardSpeed = Move / DeltaTime;
		if (FMath::IsNearlyZero(RemainingDistance, 0.1f))
		{
			bDrivingDistance = false;
			RemainingDistance = 0.0f;
			ForwardSpeed = 0.0f;
		}
	}
	else if (bTurning)
	{
		float Direction = FMath::Sign(RemainingYaw);
		float Step = CommandTurnRate * DeltaTime;
		float Rotate = FMath::Min(FMath::Abs(RemainingYaw), Step) * Direction;
		RemainingYaw -= Rotate;
		TurnRate = Rotate / DeltaTime;
		if (FMath::IsNearlyZero(RemainingYaw, 0.5f))
		{
			bTurning = false;
			RemainingYaw = 0.0f;
			TurnRate = 0.0f;
		}
	}
	else if (!FMath::IsNearlyZero(CommandSpeed))
	{
		ForwardSpeed = CommandSpeed;
	}

	if (FMath::IsNearlyZero(ForwardSpeed) && FMath::IsNearlyZero(TurnRate))
	{
		return;
	}

	FRotator NewRotation = GetActorRotation();
	NewRotation.Yaw += TurnRate * DeltaTime;
	SetActorRotation(NewRotation);

	FVector Forward = NewRotation.Vector();
	FVector Delta = Forward * ForwardSpeed * DeltaTime;
	AddActorWorldOffset(Delta, true);
}

