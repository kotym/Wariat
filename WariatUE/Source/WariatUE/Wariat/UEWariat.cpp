// Fill out your copyright notice in the Description page of Project Settings.
#include "Wariat/UEWariat.h"
#include "Components/SkeletalMeshComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInputComponent.h"
#include "ChaosWheeledVehicleMovementComponent.h"
#include "WariatUE.h"
#include "HC_SR04.h"
#include "../WariatCommon/ComMath.hpp"

// Sets default values
AUEWariat::AUEWariat()
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

