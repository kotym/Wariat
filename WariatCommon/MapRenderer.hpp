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
    Wariat,
    ScanAhead, // debug
    ScanRight, // debug
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
        transform.position /= map.GetCellSizeInCm();
        Vector2<int32_t> WariatPosOnMap(transform.position);
        Vector2<int32_t> renderedCell;
        for (renderedCell.y = -renderedMapHalfSize.y; renderedCell.y < renderedMapHalfSize.y; ++renderedCell.y)
        {
            for (renderedCell.x = -renderedMapHalfSize.x; renderedCell.x < renderedMapHalfSize.x; ++renderedCell.x)
            {
                int32_t TexturePixelIndex = (renderedCell.y + renderedMapHalfSize.y) * renderedMapSize.x + renderedCell.x + renderedMapHalfSize.x;
                EMapCellState cell = map.GetCellState(WariatPosOnMap + renderedCell);

                if (cell == EMapCellState::Invalid)
                {
                    SetRenderedMapCell(TexturePixelIndex, CellColor::OutOfBounds);
                    continue;
                }

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

        const int32_t mapWidthInCells = map.GetMapWidthInCells();

        for (int Cell : lastScanOutlineCells)
        {
            int32_t CellY = Cell / mapWidthInCells;
            int32_t CellX = Cell - CellY * mapWidthInCells;
            int32_t TexturePixelIndex = (CellY + renderedMapHalfSize.y) * renderedMapSize.x + CellX + renderedMapHalfSize.x;
            SetRenderedMapCell(TexturePixelIndex, CellColor::VisionCone);
        }

        for (Vector2<int32_t> cell : map.lastScannedAhead)
        {
            cell -= WariatPosOnMap;
            int32_t TexturePixelIndex = (cell.y + renderedMapHalfSize.y) * renderedMapSize.x + cell.x + renderedMapHalfSize.x;
            SetRenderedMapCell(TexturePixelIndex, CellColor::ScanAhead);
        }

        for (Vector2<int32_t> cell : map.lastScannedRight)
        {
            cell -= WariatPosOnMap;
            int32_t TexturePixelIndex = (cell.y + renderedMapHalfSize.y) * renderedMapSize.x + cell.x + renderedMapHalfSize.x;
            SetRenderedMapCell(TexturePixelIndex, CellColor::ScanRight);
        }
    }
};

}