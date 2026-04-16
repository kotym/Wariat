#pragma once
#include <cstdint>
#include <vector>
#include "ComMath.hpp"

enum class EMapCellState : uint8_t
{
	Unknown = 0,
	Empty,
	Wall,
	Idk,
	// Not allowed in map cell, only for logic
	Invalid,
	//Outline
};

union ByteOfCells{
	struct {
		uint8_t cell0 : 2;
		uint8_t cell1 : 2;
		uint8_t cell2 : 2;
		uint8_t cell3 : 2;
	};
	struct {
		EMapCellState cell0E : 2;
		EMapCellState cell1E : 2;
		EMapCellState cell2E : 2;
		EMapCellState cell3E : 2;
	};
	uint8_t byte;
};


class ComMap
{
public:
	struct MemoryOps
	{
        void (*zero)(uint8_t* ptr, int32_t size) = nullptr;
		uint8_t* (*reallocFn)(uint8_t* ptr, int32_t size) = nullptr;
		uint8_t* (*mallocFn)(int32_t size) = nullptr;
		void (*freeFn)(uint8_t* ptr) = nullptr;
	};

	explicit ComMap(MemoryOps memoryOps);
	~ComMap();

	void Reset();
	void UpdateMapFromScan(Vector2<int32_t> position, float rotation, float range, float coneAngle, bool wasHit, Vector2<int32_t> centerPosOffset);

	const uint8_t* GetMap() const { return map; }
	int32_t GetMapWidthInCells() const { return mapWidthInCells; }
	int32_t GetMapWidthInBytes() const { return mapWidthInBytes; }
	int32_t GetMapSizeInCells() const { return mapSizeInCells; }
	int32_t GetMapSizeInBytes() const { return mapSizeInBytes; }
	int32_t GetCellsInByte() const { return cellsInByte; }
	int32_t GetCellSizeInCm() const { return cellSizeInCm; }
	const std::vector<int>& GetLastScanOutlineCells() const { return lastScanOutlineCells; }

	void ResetOutline();

protected:
	inline void UpdateCell(int32_t x, int32_t y, EMapCellState newState);
	void ZeroMap();
	void ReallocMap();
	void MallocMap();
	void FreeMap();

protected:
	// square map of mapWidthInCells x mapWidthInCells
	uint8_t* map = nullptr;
	// number of rows and columns in map
	int32_t mapWidthInCells = 1'024;
	// number of bytes per row, mapWidthInCells / cellsInByte
	int32_t mapWidthInBytes = 256;
	// mapWidthInCells^2 
	int32_t mapSizeInCells = 1'048'576;
	// mapWidthInCells^2 / cellsInByte
	int32_t mapSizeInBytes = 262'144;
	// how many cells are in one byte
	int32_t cellsInByte = 4;
	// how big is each cell
	int32_t cellSizeInCm = 5;

	//FVector2D Position;
	//float Rotation;

	// outline cells indexes
	std::vector<int> lastScanOutlineCells;
	MemoryOps memoryOps;


};
