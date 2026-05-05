#include "Wariat/WariatUI.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"
#include "UEWariat.h"

void UWariatUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	InitializeMapTexture();
	if (MapImage != nullptr)
		MapImage->SetBrushFromTexture(MapTexture);

	AUEWariat* const Wariat = Cast<AUEWariat>(GetOwningPlayerPawn());
	if (Wariat == nullptr)
		return;
	Wariat->pureWariat.mapRenderer.wariatUI = this;
	mapRenderer = &Wariat->pureWariat.mapRenderer;
	MapTextureWidth = mapRenderer->renderedMapSize.x;
	MapTextureHeight = mapRenderer->renderedMapSize.y;
}

void UWariatUI::NativeDestruct()
{
	if (MapTexture != nullptr) MapTexture->RemoveFromRoot();
	UpdateTexture(true);
	Super::NativeDestruct();
}

void UWariatUI::InitializeMapTexture()
{
	// Get Total Pixels in Texture
	MapTextureTotalPixels = MapTextureWidth * MapTextureHeight;

	// Get Total Bytes of Texture - Each pixel has 4 bytes for RGBA
	MapTextureDataSqrtSize = MapTextureWidth * 4;

	// Initialize Texture Data Array
	MapTextureData = (Pixel*)FMemory::Malloc(MapTextureTotalPixels * sizeof(Pixel), alignof(Pixel));

	// Create Dynamic Texture Object
	MapTexture = UTexture2D::CreateTransient(MapTextureWidth, MapTextureHeight);
	MapTexture->CompressionSettings = TextureCompressionSettings::TC_VectorDisplacementmap;
	MapTexture->SRGB = 0;
	MapTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
	MapTexture->Filter = TextureFilter::TF_Nearest;
	MapTexture->AddToRoot();
	MapTexture->UpdateResource();

	//Create Update Region Struct Instance
	TextureRegion = new FUpdateTextureRegion2D(0, 0, 0, 0, MapTextureWidth, MapTextureHeight);

	FillTexture(FLinearColor::Black);
	UpdateTexture();
}

void UWariatUI::NativeTick(const FGeometry& MyGeometry, float InDeltaTime)
{
	Super::NativeTick(MyGeometry, InDeltaTime);

	UpdateTexture();
}

void UWariatUI::UpdateTexture(bool bFreeData)
{
	if (MapTexture == nullptr || MapTextureData == nullptr)
	{
		UE_LOG(LogTemp, Warning, TEXT("Dynamic Texture tried to Update Texture but it hasn't been initialized!"));
		return;
	}

	struct FUpdateTextureRegionsData
	{
		FTexture2DResource* Texture2DResource;
		FRHITexture* TextureRHI;
		int32 MipIndex;
		uint32 NumRegions;
		FUpdateTextureRegion2D* Regions;
		uint32 SrcPitch;
		uint32 SrcBpp;
		uint8* SrcData;
	};

	FUpdateTextureRegionsData* RegionData = new FUpdateTextureRegionsData;

	UTexture2D* Texture = MapTexture;

	RegionData->Texture2DResource = (FTexture2DResource*)Texture->GetResource();
	RegionData->TextureRHI = RegionData->Texture2DResource->GetTexture2DRHI();
	RegionData->MipIndex = 0;
	RegionData->NumRegions = 1;
	RegionData->Regions = TextureRegion;
	RegionData->SrcPitch = MapTextureDataSqrtSize;
	RegionData->SrcBpp = 4;
	RegionData->SrcData = (uint8*)MapTextureData;

	ENQUEUE_RENDER_COMMAND(UpdateTextureRegionsData)(
		[RegionData, bFreeData, Texture](FRHICommandListImmediate& RHICmdList)
		{
			for (uint32 RegionIndex = 0; RegionIndex < RegionData->NumRegions; ++RegionIndex)
			{
				int32 CurrentFirstMip = Texture->FirstResourceMemMip;
				if (RegionData->TextureRHI && RegionData->MipIndex >= CurrentFirstMip)
				{
					RHIUpdateTexture2D(
						RegionData->TextureRHI,
						RegionData->MipIndex - CurrentFirstMip,
						RegionData->Regions[RegionIndex],
						RegionData->SrcPitch,
						RegionData->SrcData
						+ RegionData->Regions[RegionIndex].SrcY * RegionData->SrcPitch
						+ RegionData->Regions[RegionIndex].SrcX * RegionData->SrcBpp
					);
				}
			}
			if (bFreeData) {
				delete RegionData->Regions;
				FMemory::Free(RegionData->SrcData);
			}
			delete RegionData;
		});
}

void UWariatUI::FillTexture(FLinearColor Color)
{
	if (MapTextureData == nullptr || MapTexture == nullptr) 
		return;
	for (uint32 i = 0; i < MapTextureTotalPixels; i++)
	{
		MapTextureData[i] = Color;
	}
}


void UWariatUI::SetRenderedMapCell(uint32_t cellIndex, WariatCommon::CellColor cellColor)
{
	static const FLinearColor Pink(1.f, 0.f, 0.5f, 1.f);
	const FLinearColor* Color = nullptr;
	switch (cellColor)
	{
		case WariatCommon::CellColor::Unknown:
			Color = &CellColorUnknown;
			break;
		case WariatCommon::CellColor::Empty:
			Color = &CellColorEmpty;
			break;
		case WariatCommon::CellColor::Wall:
			Color = &CellColorFull;
			break;
		case WariatCommon::CellColor::Idk:
			Color = &CellColorRatherFull;
			break;
		case WariatCommon::CellColor::Wariat:
			Color = &CellColorPlayerOutline;
			break;
		case WariatCommon::CellColor::VisionCone:
			Color = &CellColorScanOutline;
			break;
		default:
			Color = &Pink;
	}

	MapTextureData[cellIndex] = *Color;
}