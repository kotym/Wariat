// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "../WariatCommon/MapRenderer.hpp"

#include "WariatUI.generated.h"

class UImage;
class UTexture2D;

union Pixel {
	struct {
		uint8 B;
		uint8 G;
		uint8 R;
		uint8 A;
	};
	uint32 BGRA;

	Pixel() {}
	Pixel(const FLinearColor& Color) : B(Color.B * 255), G(Color.G * 255), R(Color.R * 255), A(Color.A * 255) {}
	//Pixel(uint8) : B(Color.B), G(Color.G), R(Color.R), A(Color.A) {}
};


UCLASS()
class WARIATUE_API UWariatUI : public UUserWidget
{
	GENERATED_BODY()
	
protected:
	UPROPERTY(meta=(BindWidget))
	TObjectPtr<UImage> MapImage;

	UPROPERTY()
	UTexture2D* MapTexture;

	UPROPERTY(EditDefaultsOnly)
	int32 MapTextureWidth = 256;

	UPROPERTY(EditDefaultsOnly)
	int32 MapTextureHeight = 256;

	UPROPERTY(EditAnywhere)
	FLinearColor CellColorUnknown;
	UPROPERTY(EditAnywhere)
	FLinearColor CellColorEmpty;
	UPROPERTY(EditAnywhere)
	FLinearColor CellColorFull;
	UPROPERTY(EditAnywhere)
	FLinearColor CellColorRatherFull;
	UPROPERTY(EditAnywhere)
	FLinearColor CellColorScanOutline;
	UPROPERTY(EditAnywhere)
	FLinearColor CellColorPlayerOutline;

	FUpdateTextureRegion2D* TextureRegion;

	// Array that contains the Texture Data
	Pixel* MapTextureData;

	// Texture Data Sqrt Size
	uint32 MapTextureDataSqrtSize;

	// Total Count of Pixels in Texture
	uint32 MapTextureTotalPixels;

public:
	void UpdateTexture(bool bFreeData = false);

protected:
	void NativeOnInitialized() override;
	void NativeDestruct() override;

	void InitializeMapTexture();

	void NativeTick(const FGeometry& MyGeometry, float InDeltaTime) override;

	void FillTexture(FLinearColor Color);

public:

	// MapRenderer

	void SetRenderedMapCell(uint32_t cellIndex, WariatCommon::CellColor cellColor);

	class UEMapRenderer* mapRenderer = nullptr;
};


class UEMapRenderer : public WariatCommon::MapRenderer<UEMapRenderer>
{
	friend UWariatUI;
public:
	void SetRenderedMapCell(uint32_t cellIndex, WariatCommon::CellColor cellColor)
	{
		wariatUI->SetRenderedMapCell(cellIndex, cellColor);
	}

private:

	UWariatUI* wariatUI = nullptr;
};
