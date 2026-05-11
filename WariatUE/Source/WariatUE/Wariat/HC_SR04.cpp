// Fill out your copyright notice in the Description page of Project Settings.


#include "Wariat/HC_SR04.h"
#include "WariatUE.h"
#include "Components/StaticMeshComponent.h"
#include <Math/UnrealMathUtility.h>
#include <Components/PrimitiveComponent.h>
#include "Engine/OverlapResult.h"

// Sets default values for this component's properties
UHC_SR04::UHC_SR04()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = true;
	
	CollisionShape = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CollisionShape"));
	CollisionShape->SetupAttachment(this);
	CollisionShape->SetCollisionEnabled(ECollisionEnabled::NoCollision);// ECollisionEnabled::QueryOnly);
	//CollisionShape->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	//CollisionShape->SetCollisionResponseToChannel(ECollisionChannel_HC_SR04, ECollisionResponse::ECR_Overlap);
	//CollisionShape->SetGenerateOverlapEvents(true);

	Mesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	Mesh->SetupAttachment(this);
	// ...
}


// Called when the game starts
void UHC_SR04::BeginPlay()
{
	Super::BeginPlay();

	// ...
	//StartTrace();
}

void UHC_SR04::OnRegister()
{
    Super::OnRegister();
    if (Mesh)
    {
        Mesh->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
        CollisionShape->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
    }
}

//void UHC_SR04::TraceCallback(const FTraceHandle& InTraceHandle, FTraceDatum& InTraceDatum)
//{
//	bDuringTrace = false;
//	StartTrace();
//	const FVector ComponentLocation = GetComponentLocation();
//	const FVector ForwardDirection = GetForwardVector();
//	float Cos = cosf(FMath::DegreesToRadians(ConeAngle));
//	DrawDebugLine(GetWorld(), ComponentLocation, ComponentLocation + ForwardDirection * 100, FColor::Magenta);
//
//	int Nearest = -1;
//	float NearestDist = FLT_MAX;
//
//	const int32 HitNum = InTraceDatum.OutHits.Num();
//	for (int i = 0; i < HitNum; ++i)
//	{
//		const FVector Direction = InTraceDatum.OutHits[i].ImpactPoint - ComponentLocation;
//		if (Direction.GetSafeNormal().Dot(ForwardDirection) >= Cos && Direction.SquaredLength() < NearestDist)
//		{
//			DrawDebugLine(GetWorld(), ComponentLocation, Direction * 100, FColor::Cyan);
//			Nearest = i;
//		}
//		else
//		{
//			DrawDebugLine(GetWorld(), ComponentLocation, Direction * 100, FColor::Orange);
//
//		}
//	}
//
//	UE_LOG(LogTemp, Log, TEXT("Trace Completed"));
//	if (Nearest < 0) return;
//	LastDetectionDist = sqrtf(NearestDist);
//	LastDetection = InTraceDatum.OutHits[Nearest].ImpactPoint;
//
//}


// Called every frame
void UHC_SR04::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	//SphereConeTrace();
}

void UHC_SR04::SphereConeTrace()
{
	UWorld* const World = GetWorld();
	ensureMsgf(World, TEXT("World is nullptr  %s  %d"), ANSI_TO_TCHAR(__FILE__), __LINE__);
	if (!World) return;

	FQuat Quat = GetComponentQuat();
	FVector Forward = Quat.GetForwardVector();
	FVector Start = GetComponentLocation();
	FVector End = Start + Forward * MaxDetectionDist;
	
	//DrawDebugLine(GetWorld(), Start, End, FColor::Magenta);

	FCollisionQueryParams Params;
	Params.AddIgnoredActor(GetOwner());

	//DrawDebugCone(World, Start, Forward, 400.f, FMath::DegreesToRadians(ConeAngle/2), FMath::DegreesToRadians(ConeAngle/2), 10, FColor::Red);
	float sinAngle = sin(GetDetectionAngleRad() / 2);
	float Dist = 4.f, Radius = Dist / (1 + sinAngle);

	int32 SpheresChecked = 0;

	FVector Nearest = End;
	float NearestDist = MaxDetectionDist * MaxDetectionDist;

	int8 afterHit = 0;

	while (Dist < MaxDetectionDist && afterHit < 2)
	{
		++SpheresChecked;
		FVector SpherePos = Forward * Dist + Start;
		TArray<FHitResult> Hits;
		World->SweepMultiByChannel(Hits, SpherePos, SpherePos + Forward, FQuat::Identity, ECollisionChannel_HC_SR04, FCollisionShape::MakeSphere(Radius), Params);
		//DrawDebugSphere(World, SpherePos, Radius, 10, FColor::Orange);
		if (afterHit > 0) ++afterHit;

		const int32 HitNum = Hits.Num();
		for (int i = 0; i < HitNum; ++i)
		{
			++afterHit;
			//DrawDebugSphere(World, Hits[i].ImpactPoint, 7.f, 10, FColor::Red);
			const FVector HitDist = Hits[i].ImpactPoint - Start;
			const float Dist2 = HitDist.SquaredLength();
			if (Dist2 < NearestDist)
			{
				NearestDist = Dist2;
				Nearest = Hits[i].ImpactPoint;
			}

			//DrawDebugLine(GetWorld(), Start, Hits[i].ImpactPoint, FColor::Orange);
		}

		Dist += Radius;
		Radius = Dist * sinAngle;
	}

	//if (GEngine)
	//	GEngine->AddOnScreenDebugMessage(123243, 5, FColor::Emerald, FString::Printf(TEXT("SpheresChecked: %d"), SpheresChecked));

	bLastDetectionWasHit = afterHit > 0;
	//if (NearestDist == FLT_MAX) return;
	LastDetectionDist = sqrtf(NearestDist);
	//LastDetection = Nearest;
	DrawDebugLine(GetWorld(), Start, Nearest, FColor::Cyan, false, -1, 0, 1);
	//DrawDebugSphere(World, Nearest, 10.f, 10, FColor::Green);	
}

//void UHC_SR04::ConeSweep()
//{
//	UWorld* const World = GetWorld();
//	ensureMsgf(World, TEXT("World is nullptr  %s  %d"), ANSI_TO_TCHAR(__FILE__), __LINE__);
//	if (!World) return;
//
//	FQuat Quat = CollisionShape->GetComponentQuat();
//	FVector Forward = -Quat.GetRightVector();
//	FVector Start = CollisionShape->GetComponentLocation();
//	FVector End = Start + Forward * MaxDetectionDist;
//
//	//DrawDebugCone(World, Start, Forward, 400.f, FMath::DegreesToRadians(ConeAngle/2), FMath::DegreesToRadians(ConeAngle/2), 10, FColor::Red);
//
//	TArray<FHitResult> Hits;
//	FComponentQueryParams Params;
//
//	Params.AddIgnoredActor(GetOwner());
//	World->ComponentSweepMultiByChannel(Hits, CollisionShape.Get(), Start, Start, Quat, ECollisionChannel_HC_SR04, Params);
//
//	DrawDebugLine(GetWorld(), Start, End, FColor::Magenta);
//
//	FVector Nearest = End;
//	float NearestDist = MaxDetectionDist * MaxDetectionDist;
//	bLastDetectionWasHit = false;
//
//	const int32 HitNum = Hits.Num();
//	for (int i = 0; i < HitNum; ++i)
//	{
//		bLastDetectionWasHit = true;
//		DrawDebugSphere(World, Hits[i].ImpactPoint, 7.f, 10, FColor::Red);
//		const FVector Dist = Hits[i].ImpactPoint - Start;
//		const float Dist2 = Dist.SquaredLength();
//		if (Dist2 < NearestDist)
//		{
//			NearestDist = Dist2;
//			Nearest = Hits[i].ImpactPoint;
//		}
//
//		DrawDebugLine(GetWorld(), Start, Hits[i].ImpactPoint, FColor::Orange);
//	}
//
//	UE_LOG(LogTemp, Log, TEXT("Trace Completed"));
//	//if (NearestDist == FLT_MAX) return;
//	LastDetectionDist = sqrtf(NearestDist);
//	//LastDetection = Nearest;
//	DrawDebugLine(GetWorld(), Start, Nearest, FColor::Cyan, false, -1, 0, 1);
//	DrawDebugSphere(World, Nearest, 10.f, 10, FColor::Green);
//}
//
//void UHC_SR04::StartTrace()
//{
//	UWorld* const World = GetWorld();
//    ensureMsgf(World, TEXT("World is nullptr  %s  %d"), ANSI_TO_TCHAR(__FILE__), __LINE__);
//	if (!World) return;
//
//	ensureMsgf(!bDuringTrace, TEXT("Starting a trace during a trace HC_SR04"));
//	bDuringTrace = true;
//	FCollisionQueryParams Params;
//	Params.AddIgnoredActor(GetOwner());
//	//FCollisionResponseParams ResponseParams;
//	FTraceDelegate Delegate = FTraceDelegate::CreateUObject(this, &UHC_SR04::TraceCallback);
//	World->AsyncSweepByChannel(EAsyncTraceType::Multi, GetComponentLocation(), FVector::OneVector * 400, FQuat::Identity, ECollisionChannel_HC_SR04, FCollisionShape::MakeSphere(400), Params, FCollisionResponseParams::DefaultResponseParam, &Delegate);
//}
//
