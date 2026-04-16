#include "Wariat/WariatUI.h"
#include "Engine/Texture2D.h"
#include "Components/Image.h"
#include "RHICommandList.h"
#include "Rendering/Texture2DResource.h"
#include "Wariat.h"

void UWariatUI::NativeOnInitialized()
{
	Super::NativeOnInitialized();
	InitializeMapTexture();
	if (MapImage != nullptr)
		MapImage->SetBrushFromTexture(MapTexture);
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

	if (0)
	{
		static FVector Color;

		float a = 1, b = 1, c = 1;
		Color.X += InDeltaTime * 0.5 * a;
		Color.Y += InDeltaTime * 1 * b;
		Color.Z += InDeltaTime * 1.5 * c;

		if (Color.X > 255 || Color.X < 0) a = -a;
		if (Color.Y > 255 || Color.Y < 0) b = -b;
		if (Color.Z > 255 || Color.Z < 0) c = -c;

		FillTexture(FLinearColor(Color));
		UpdateTexture();
	}
	else
	{
		UpdateMapFromScan();
		UpdateTexture();
	}
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

void UWariatUI::UpdateMapFromScan()
{
	AWariat* const Wariat = Cast<AWariat>(GetOwningPlayerPawn());
	if (Wariat == nullptr || MapTextureData == nullptr || MapTexture == nullptr) 
		return;
	const PureMap& pureMap = Wariat->GetPureMap();
	const ByteOfCells* const map = (const ByteOfCells*)pureMap.GetMap();
	if (map == nullptr) return;
	int32 mapWidthInCells = pureMap.GetMapWidthInCells();
	int32 mapWidthInBytes = pureMap.GetMapWidthInBytes();
	int32 mapCellsInByte = pureMap.GetCellsInByte();
	int32 mapByteSize = mapWidthInBytes * mapWidthInBytes;

	float Rotation = FMath::DegreesToRadians(Wariat->GetActorRotation().Yaw);
	//FVector2D Forward(FMath::Cos(Rotation), FMath::Sin(Rotation));
	//FVector2D DetectionVector = Forward * Dist;
	//FVector2D ScaledDetectionVector = DetectionVector / 5;
	//FIntVector2 MapVector(ScaledDetectionVector.X, ScaledDetectionVector.Y);
	FVector2D Pos(FVector2D(Wariat->GetActorLocation()) - Wariat->GetZeroLocation());
	Pos /= pureMap.GetCellSizeInCm();
	FIntVector2 WariatPosOnMap(Pos.X, Pos.Y);

	int32 MapTextureHeightHalf = MapTextureHeight / 2;
	int32 MapTextureWidthHalf = MapTextureWidth / 2;

	for (int32 y = -MapTextureHeightHalf; y < MapTextureHeightHalf; y++)
	{
		int32 mapY = Pos.Y + y;
		mapY += mapWidthInCells / 2;
		for (int32 x = -MapTextureWidthHalf; x < MapTextureWidthHalf; ++x)
		{
			int32 mapX = Pos.X + x;
			mapX += mapWidthInCells / 2;
			int32 TexturePixelIndex = (y + MapTextureHeightHalf)* MapTextureWidth + x + MapTextureWidthHalf;
			if (mapY < 0 || mapY >= mapWidthInCells || mapX < 0 || mapX >= mapWidthInCells)
			{
				MapTextureData[TexturePixelIndex] = FLinearColor::Black;
				continue;
			}

			int32 bytePos = mapX / mapCellsInByte + mapY * mapWidthInBytes;

			ByteOfCells byteOfCells = map[bytePos];
			int32 inBytePos = (mapCellsInByte - mapX % mapCellsInByte - 1) * 8 / mapCellsInByte;
			EMapCellState cell = (EMapCellState)(byteOfCells.byte >> inBytePos & 0b11);

			static const FLinearColor Pink(1.f, 0.f, 0.5f, 1.f);
			const FLinearColor* Color = nullptr;
			switch (cell)
			{
				case EMapCellState::Unknown:
					Color = &CellColorUnknown;
					break;
				case EMapCellState::Empty:
					Color = &CellColorEmpty;
					break;
				case EMapCellState::Wall:
					Color = &CellColorFull;
					break;
				case EMapCellState::Idk:
					Color = &CellColorRatherFull;
					break;
				default:
					Color = &Pink;
			}

			MapTextureData[TexturePixelIndex] = *Color;
		}
	}

	int32 PlayerCircleRadius = 8;
	for (int32 y = -PlayerCircleRadius; y < PlayerCircleRadius; y++)
	{
		for (int32 x = -MapTextureWidthHalf; x < MapTextureWidthHalf; ++x)
		{
			if (x * x + y * y <= PlayerCircleRadius * PlayerCircleRadius)
			{
				int32 TexturePixelIndex = (y + MapTextureHeightHalf) * MapTextureWidth + x + MapTextureWidthHalf;
				MapTextureData[TexturePixelIndex] = CellColorPlayerOutline;
			}
		}
	}

	const std::vector<int>& lastScanOutlineCells = pureMap.GetLastScanOutlineCells();

	// Outline start point should be calculated from the HC_SR04 location not players

	for (int Cell : lastScanOutlineCells)
	{
		int32 CellY = Cell / mapWidthInCells;
		int32 CellX = Cell - CellY * mapWidthInCells;
		int32 TextureY = CellY;
		int32 TextureX = CellX;
		int32 TexturePixelIndex = (TextureY + MapTextureHeightHalf) * MapTextureWidth + TextureX + MapTextureWidthHalf;
		MapTextureData[TexturePixelIndex] = CellColorScanOutline;
	}
}