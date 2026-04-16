#pragma once

#include "CoreMinimal.h"
#include "Components/SceneComponent.h"
#include "HC_SR04.generated.h"


UCLASS( Blueprintable, BlueprintType, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class WARIATUE_API UHC_SR04 : public USceneComponent
{
	GENERATED_BODY()

protected:
	//UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(MakeEditWidget))
	//FVector TraceStart;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> CollisionShape;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	TObjectPtr<UStaticMeshComponent> Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ConeAngle = 30.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MaxDetectionDist = 400.f;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FVector LastDetection;
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float LastDetectionDist = 0;

	
	//bool bDuringTrace = false;
	bool bLastDetectionWasHit = false;

public:	
	// Sets default values for this component's properties
	UHC_SR04();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	//void TraceCallback(const FTraceHandle& InTraceHandle, FTraceDatum& InTraceDatum);

    virtual void OnRegister() override;

public:	
	// Called every frame
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	//void StartTrace();

	inline float GetLastDetectionDist() const { 
		//ensureMsgf(!bDuringTrace, TEXT("Accessing LastDetectionDist during a trace  %s  %d"), ANSI_TO_TCHAR(__FILE__), __LINE__); 
		return LastDetectionDist; 
	}

	inline bool WasLastDetectionHit() const {
		return bLastDetectionWasHit;
	}

	inline float GetDetectionAngleDg() const { return ConeAngle; }
	inline float GetDetectionAngleRad() const { return FMath::DegreesToRadians(ConeAngle); }

public:
	void SphereConeTrace();
	//void ConeSweep();

};
