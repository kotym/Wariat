#pragma once
#include "ComMap.hpp"
#include "ComMath.hpp"

namespace WariatCommon
{

enum class CellColor : uint8_t
{
    None,
    OutOfBounds,
    Unknown,
    Empty,
    Wall,
    Idk,
    VisionCone,
    Wariat
};

static constexpr CellColor MapCellStateToCellColor(EMapCellState mapCellState)
{
    switch (mapCellState)
    {
        case EMapCellState::Unknown: return CellColor::Unknown;
        case EMapCellState::Empty: return CellColor::Empty;
        case EMapCellState::Wall: return CellColor::Wall;
        case EMapCellState::Idk: return CellColor::Idk;
        case EMapCellState::Invalid: return CellColor::None;
    }
}

template<class Derived>
class MapRenderer
{
public:

    const Vector2<uint16_t> renderedMapSize = {256, 256};
    const Vector2<uint16_t> renderedMapHalfSize = {128, 128};

public:
    MapRenderer() : renderedMapHalfSize(renderedMapSize / 2)
    {
    }

    void SetRenderedMapCell(uint32_t cellIndex, CellColor cellColor)
    {
        static_cast<Derived*>(this)->SetRenderedMapCell(cellIndex, cellColor);
    }

    void RenderMap(Transform transform, ComMap& map)
    {
        const ByteOfCells* const byteMap = (const ByteOfCells*)map.GetMap();
        if (byteMap == nullptr) return;
        int32_t mapWidthInCells = map.GetMapWidthInCells();
        int32_t mapWidthInBytes = map.GetMapWidthInBytes();
        int32_t mapCellsInByte = map.GetCellsInByte();
        int32_t mapByteSize = mapWidthInBytes * mapWidthInBytes;

        //FVector2D Forward(FMath::Cos(Rotation), FMath::Sin(Rotation));
        //FVector2D DetectionVector = Forward * Dist;
        //FVector2D ScaledDetectionVector = DetectionVector / 5;
        //FIntVector2 MapVector(ScaledDetectionVector.X, ScaledDetectionVector.Y);
        transform.position /= map.GetCellSizeInCm();
        Vector2<int32_t> WariatPosOnMap(transform.position);
        
        for (int32_t y = -renderedMapHalfSize.y; y < renderedMapHalfSize.y; y++)
        {
            int32_t mapY = WariatPosOnMap.y + y;
            mapY += mapWidthInCells / 2;
            for (int32_t x = -renderedMapHalfSize.x; x < renderedMapHalfSize.x; ++x)
            {
                int32_t mapX = WariatPosOnMap.x + x;
                mapX += mapWidthInCells / 2;
                int32_t TexturePixelIndex = (y + renderedMapHalfSize.y) * renderedMapSize.x + x + renderedMapHalfSize.x;
                if (mapY < 0 || mapY >= mapWidthInCells || mapX < 0 || mapX >= mapWidthInCells)
                {
                    SetRenderedMapCell(TexturePixelIndex, CellColor::OutOfBounds);
                    continue;
                }

                int32_t bytePos = mapX / mapCellsInByte + mapY * mapWidthInBytes;

                ByteOfCells byteOfCells = byteMap[bytePos];
                int32_t inBytePos = (mapCellsInByte - mapX % mapCellsInByte - 1) * 8 / mapCellsInByte;
                EMapCellState cell = (EMapCellState)(byteOfCells.byte >> inBytePos & 0b11);

                SetRenderedMapCell(TexturePixelIndex, MapCellStateToCellColor(cell));
            }
        }

        int32_t PlayerCircleRadius = 8;
        for (int32_t y = -PlayerCircleRadius; y < PlayerCircleRadius; y++)
        {
            for (int32_t x = -renderedMapHalfSize.x; x < renderedMapHalfSize.x; ++x)
            {
                if (x * x + y * y <= PlayerCircleRadius * PlayerCircleRadius)
                {
                    int32_t TexturePixelIndex = (y + renderedMapHalfSize.y) * renderedMapSize.x + x + renderedMapHalfSize.x;
                    SetRenderedMapCell(TexturePixelIndex, CellColor::Wariat);
                }
            }
        }

        const std::vector<int>& lastScanOutlineCells = map.GetLastScanOutlineCells();

        // TODO Outline start point should be calculated from the HC_SR04 location not players

        for (int Cell : lastScanOutlineCells)
        {
            int32_t CellY = Cell / mapWidthInCells;
            int32_t CellX = Cell - CellY * mapWidthInCells;
            int32_t TexturePixelIndex = (CellY + renderedMapHalfSize.y) * renderedMapSize.x + CellX + renderedMapHalfSize.x;
            SetRenderedMapCell(TexturePixelIndex, CellColor::VisionCone);
        }
    }
};

}