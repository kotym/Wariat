// Fill out your copyright notice in the Description page of Project Settings.
#include "Wariat/UEWariat.h"
#include "Components/StaticMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "WariatUE.h"
#include "HC_SR04.h"
#include "../WariatCommon/ComMath.hpp"


//void ProcessEventUE(WariatCommon::PacketPayloadType payloadType, void* payload)
//{
//	pureWariat->ProcessEvent(payloadType, payload);
//}

void AUEWariat::ProcessCommand(WariatCommon::PacketPayloadType payloadType, void* payload)
{
	switch (payloadType)
	{
		case WariatCommon::PacketPayloadType::Stop:
			StopKinematic();
			break;
		case WariatCommon::PacketPayloadType::MoveForward:
			MoveForward(static_cast<WariatCommon::Payload::MoveForward*>(payload)->distanceCm, MaxSpeedCmPerSec);
			break;
		case WariatCommon::PacketPayloadType::Rotate:
			Rotate(static_cast<WariatCommon::Payload::Rotate*>(payload)->angle, MaxTurnRateDegPerSec);
			break;
		//case WariatCommon::PacketPayloadType::RotateAndMove:
		//{
		//	WariatCommon::Payload::RotateAndMove* payload = static_cast<WariatCommon::Payload::RotateAndMove*>(payload);
		//	Rotate(payload->angle, MaxTurnRateDegPerSec, false);
		//	//MoveForward(payload->distanceCm, MaxSpeedCmPerSec);
		//	break;
		//}
		case WariatCommon::PacketPayloadType::BlinkToggle:
			//hLED1.toggle();
			//Serial.printf("blink...");
			break;
	}
}

// Sets default values
AUEWariat::AUEWariat()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
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
	pureWariat.comInterface.ueWariat = this;
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

	//FVector2D pos(FVector2D(GetActorLocation()) - ZeroLocation);
	//pureWariat.transform.position = { (float)pos.X, (float)pos.Y };
	//pureWariat.transform.rotation = FMath::DegreesToRadians(GetActorRotation().Yaw - ZeroRotation);
	pureWariat.Update();

	static WariatCommon::EState LastState = WariatCommon::EState::None;
	if (LastState != pureWariat.navi.state)
	{
		LastState = pureWariat.navi.state;
		FString text;
		switch (pureWariat.navi.state)
		{
			case WariatCommon::EState::None: text = TEXT("None"); break;
			case WariatCommon::EState::Start: text = TEXT("Start"); break;
			case WariatCommon::EState::SearchForWall: text = TEXT("SearchForWall"); break;
			case WariatCommon::EState::DriveToWall: text = TEXT("DriveToWall"); break;
			case WariatCommon::EState::DriveAlongWall: text = TEXT("DriveAlongWall"); break;
			case WariatCommon::EState::DriveAround: text = TEXT("DriveAround"); break;
		}

		GEngine->AddOnScreenDebugMessage(-1, 60, FColor::Red, text);
	}

	FString text;
	switch (pureWariat.driveState)
	{
		case WariatCommon::EDriveState::None: text = TEXT("None"); break;
		case WariatCommon::EDriveState::DriveTo: text = TEXT("DriveTo"); break;
		case WariatCommon::EDriveState::LookAt: text = TEXT("LookAt"); break;
		case WariatCommon::EDriveState::Rotate: text = TEXT("Rotate"); break;
		case WariatCommon::EDriveState::MoveForward: text = TEXT("MoveForward"); break;
		case WariatCommon::EDriveState::RotateAndDrive: text = TEXT("RotateAndDrive"); break;
		default:
			break;
	}
	GEngine->AddOnScreenDebugMessage(12344321, 60, pureWariat.bDriving? FColor::Green : FColor::Emerald, FString::Printf(TEXT("DriveState: %s"), *text));


	GEngine->AddOnScreenDebugMessage(200000000, 60, FColor::Green, FString::Printf(TEXT("transform: x: %f y: %f r: %f"), pureWariat.transform.position.x, pureWariat.transform.position.y, pureWariat.transform.rotation));
	GEngine->AddOnScreenDebugMessage(100000000, 60, FColor::Orange, FString::Printf(TEXT("destination: x: %f y: %f "), pureWariat.navi.destination.x, pureWariat.navi.destination.y));
	FVector destination(pureWariat.navi.destination.x, pureWariat.navi.destination.y, 0);
	FRotator rot(0, ZeroRotation, 0);
	FVector destination0 = rot.RotateVector(destination);
	destination0.X += ZeroLocation.X;
	destination0.Y += ZeroLocation.Y;
	DrawDebugSphere(GetWorld(), destination0, 10, 10, FColor::Purple, false, 10, 0, 2);
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
	ZeroRotation = GetActorRotation().Yaw;
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

		SendEvent(WariatCommon::Payload::HcSr04Reading(i++, Dist));
	}
}

void AUEWariat::MoveForward(float DistanceCm, float SpeedCmPerSec, bool bFinishCallback)
{
	bcallbackOnMoveFinished = bFinishCallback;
	StopKinematic();
	bDrivingDistance = true;//!FMath::IsNearlyZero(DistanceCm);
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

void AUEWariat::Rotate(float Angle, float TurnRateDegPerSec, bool bFinishCallback)
{
	bcallbackOnMoveFinished = bFinishCallback;
	StopKinematic();
	Angle = FMath::RadiansToDegrees(Angle);
	bTurning = true;//!FMath::IsNearlyZero(Angle);
	RemainingYaw = Angle;
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
			if (bcallbackOnMoveFinished) SendEvent(WariatCommon::Payload::MoveFinished());
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
			if (bcallbackOnMoveFinished) SendEvent(WariatCommon::Payload::RotationFinished());
		}
	}
	//else if (!FMath::IsNearlyZero(CommandSpeed))
	//{
	//	ForwardSpeed = CommandSpeed;
	//}
	//else
	//if (FMath::IsNearlyZero(ForwardSpeed) && FMath::IsNearlyZero(TurnRate))
	//{
	//	return;
	//}

	FRotator NewRotation = GetActorRotation();
	float RotDelta = TurnRate * DeltaTime;
	NewRotation.Yaw += RotDelta;
	SetActorRotation(NewRotation);

	FVector Forward = NewRotation.Vector();
	FVector Delta = Forward * ForwardSpeed * DeltaTime;
	AddActorWorldOffset(Delta, true);
	
	SendEvent(WariatCommon::Payload::OdometryReading(ForwardSpeed > 0 ? Delta.Length() : -Delta.Length(), FMath::DegreesToRadians(RotDelta)));
}

