#include "ComMap.hpp"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <cstring>

namespace
{
void StdZero(uint8_t* ptr, int32_t size)
{
	std::memset(ptr, 0, static_cast<std::size_t>(size));
}

uint8_t* StdRealloc(uint8_t* ptr, int32_t size)
{
	return static_cast<uint8_t*>(std::realloc(ptr, static_cast<std::size_t>(size)));
}

uint8_t* StdMalloc(int32_t size)
{
	return static_cast<uint8_t*>(std::malloc(static_cast<std::size_t>(size)));
}

void StdFree(uint8_t* ptr)
{
	std::free(ptr);
}

ComMap::MemoryOps BuildDefaultMemoryOps()
{
	ComMap::MemoryOps ops;
	ops.zero = &StdZero;
	ops.reallocFn = &StdRealloc;
	ops.mallocFn = &StdMalloc;
	ops.freeFn = &StdFree;
	return ops;
}
}

ComMap::ComMap()
	: ComMap(BuildDefaultMemoryOps())
{

}

ComMap::ComMap(MemoryOps memoryOps)
	: memoryOps(memoryOps)
{
}

ComMap::~ComMap()
{
	FreeMap();
}

void ComMap::ZeroMap()
{
	if (map != nullptr && memoryOps.zero != nullptr)
	{
		memoryOps.zero(map, mapSizeInBytes);
	}
}

void ComMap::ReallocMap()
{
	if (memoryOps.reallocFn != nullptr)
	{
		map = memoryOps.reallocFn(map, mapSizeInBytes);
	}
}

void ComMap::MallocMap()
{
	if (memoryOps.mallocFn != nullptr)
	{
		map = memoryOps.mallocFn(mapSizeInBytes);
	}
}

void ComMap::FreeMap()
{
	if (map != nullptr && memoryOps.freeFn != nullptr)
	{
		memoryOps.freeFn(map);
		map = nullptr;
	}
}

void ComMap::Reset()
{
	if (map == nullptr)
	{
		mapSizeInCells = mapWidthInCells * mapWidthInCells;
		mapSizeInBytes = mapSizeInCells / cellsInByte;
		MallocMap();
	}
	else if (mapWidthInCells * mapWidthInCells != mapSizeInCells)
	{
		mapSizeInCells = mapWidthInCells * mapWidthInCells;
		mapSizeInBytes = mapSizeInCells / cellsInByte;
		ReallocMap();
	}

	mapWidthInBytes = mapWidthInCells / cellsInByte;

	ZeroMap();
	//Position = FVector2D::ZeroVector;
	//Rotation = 0;
}

void ComMap::ResetOutline()
{
	lastScanOutlineCells.clear();
}

inline void ComMap::UpdateCell(int32_t x, int32_t y, EMapCellState newState)
{
	x += mapWidthInCells / 2;
	y += mapWidthInCells / 2;
	if (map == nullptr || x > mapWidthInCells || x < 0 || y > mapWidthInCells || y < 0 || newState == EMapCellState::Invalid) return;
	int32_t bytePos = x / cellsInByte + y * mapWidthInBytes;
	int32_t inBytePos = (cellsInByte - x % cellsInByte - 1) * 8 / cellsInByte;
	uint8_t& cellGroup = map[bytePos];
	EMapCellState cell = (EMapCellState)(cellGroup >> inBytePos & 0b11);

	//if (newState == EMapCellState::Outline)
	//{
	//	newState = EMapCellState::Empty;
	//	lastScanOutlineCells.push_back()
	//}

	switch (cell)
	{
		case EMapCellState::Wall:
		case EMapCellState::Unknown:
			cell = newState;
			break;
		default:
			return;
	}

	// Safety may be turned off in produciton
	cell = (EMapCellState)((uint8_t)cell & 0b11);
	//if ()

	uint8_t mask = 0b11 << inBytePos;
	cellGroup = (cellGroup & ~mask) | ((uint8_t)cell << inBytePos);
}

void ComMap::UpdateMapFromScan(Vector2<int32_t> position, float rotation, float range, float coneAngle, bool wasHit, Vector2<int32_t> centerPosOffset)
{
	constexpr float kPi = 3.14159265358979323846f;

	lastScanOutlineCells.reserve(lastScanOutlineCells.size() + range * 0.6f);

	float outerRange = range + wasHit;
	float rangeSq = range * range;
	float outerRangeSq = outerRange * outerRange;
	float innerRange = range - 1;
	float innerRangeSq = innerRange * innerRange;

	// Kąty ramion
	float startAngleRad = rotation - (coneAngle / 2.0f);
	float endAngleRad = rotation + (coneAngle / 2.0f);

	float sinStart = std::sin(startAngleRad);
	float cosStart = std::cos(startAngleRad);
	float sinEnd = std::sin(endAngleRad);
	float cosEnd = std::cos(endAngleRad);

	// 1. Wyznaczenie Bounding Boxa (AABB) dla całego wycinka
	float p1x = outerRange * cosStart, p1y = outerRange * sinStart;
	float p2x = outerRange * cosEnd, p2y = outerRange * sinEnd;

	float bboxMinX = std::min({ 0.0f, p1x, p2x });
	float bboxMaxX = std::max({ 0.0f, p1x, p2x });

	auto containsAngle = [startAngleRad, endAngleRad, kPi](float targetAngle) {
		float twoPI = 2.0f * kPi;
		return (targetAngle >= startAngleRad && targetAngle <= endAngleRad) ||
			(targetAngle + twoPI >= startAngleRad && targetAngle + twoPI <= endAngleRad) ||
			(targetAngle - twoPI >= startAngleRad && targetAngle - twoPI <= endAngleRad);
		};

	// Ekstrema okręgu w poziomie (przecięcie osi X)
	if (containsAngle(0.0f)) bboxMaxX = outerRange;
	if (containsAngle(kPi)) bboxMinX = -outerRange;

	float bboxMinY = std::min({ 0.0f, p1y, p2y });
	float bboxMaxY = std::max({ 0.0f, p1y, p2y });

	// Przecięcie okręgu z osią Y
	if (containsAngle(kPi / 2.0f)) bboxMaxY = outerRange;          // 90 deg
	if (containsAngle(3.0f * kPi / 2.0f)) bboxMinY = -outerRange;  // 270 deg

	int scanStartY = std::floor(bboxMinY);
	int scanEndY = std::ceil(bboxMaxY);

	// Zoptymalizowane współczynniki cotangens dla krawędzi (uniknięcie wielokrotnego dzielenia w pętli oraz dzielenia przez 0)
	const float EPSILON = 1e-5f;
	float cotStart = std::abs(sinStart) > EPSILON ? (cosStart / sinStart) : 0.0f;
	float cotEnd = std::abs(sinEnd) > EPSILON ? (cosEnd / sinEnd) : 0.0f;
	
	auto IsCellInSector = [=, this](float x, float y, int cellIndex) -> EMapCellState {
		// 1. Test promienia
		const float distSq = x * x + y * y;
		
		if (distSq > outerRangeSq) return EMapCellState::Invalid;
		

		// 2. Test kąta (iloczyn wektorowy 2D)
		// Zakładamy, że wycinek ma rozpiętość < 180 stopni
		float startArmHalfPlane = x * sinStart - y * cosStart;
		float endArmHalfPlane = x * sinEnd - y * cosEnd;

		bool afterStartLine = (startArmHalfPlane <= 0.0f);
		bool beforeEndLine = (endArmHalfPlane >= 0.0f);

		bool afterStartOutline = (startArmHalfPlane <= 1.0f);
		bool beforeEndOutline = (endArmHalfPlane >= -1.0f);
		

		EMapCellState state = EMapCellState::Invalid;
		if (distSq > rangeSq)
		{
			//if (afterStartLine && beforeEndLine)
			if (afterStartOutline && beforeEndOutline)
			{
				state = EMapCellState::Wall;

			}
		}
		else if (distSq > innerRangeSq) 
		{
			if (afterStartOutline && beforeEndOutline)
			{
				lastScanOutlineCells.push_back(cellIndex);
				state = EMapCellState::Invalid;
			}
			else if (afterStartLine && beforeEndLine)
				state = EMapCellState::Empty;
		}
		else if (afterStartLine && beforeEndLine)
		{
			state = EMapCellState::Empty;
		}
		else if (afterStartOutline && beforeEndOutline)
		{
			lastScanOutlineCells.push_back(cellIndex);
			state = EMapCellState::Invalid;
		}

		return state;
	};


	// 2. Pętla Scanline
	for (int y = scanStartY; y <= scanEndY; ++y) {
		float py = y + 0.5f; // Środek kwadratu w pionie

		// Zabezpieczenie przed pierwiastkiem z liczby ujemnej z powodu precyzji zmiennoprzecinkowej
		if (std::abs(py) >= outerRange) continue;

		// Wyznaczamy matematyczny zasięg X dla okręgu na tej wysokości
		float circleXLimit = std::sqrt(outerRangeSq - py * py);

		float rowMinX = -circleXLimit;
		float rowMaxX = circleXLimit;

		// Zawężenie na podstawie krawędzi wycinka (korzysta z uprzednio wyznaczonego cotangensa)
		if (sinStart > EPSILON) rowMaxX = std::min(rowMaxX, py * cotStart);
		else if (sinStart < -EPSILON) rowMinX = std::max(rowMinX, py * cotStart);
		else if (-py * cosStart > 0.0f) rowMinX = circleXLimit + 1.0f; // Poza okręgiem

		if (sinEnd > EPSILON) rowMinX = std::max(rowMinX, py * cotEnd);
		else if (sinEnd < -EPSILON) rowMaxX = std::min(rowMaxX, py * cotEnd);
		else if (-py * cosEnd < 0.0f) rowMinX = circleXLimit + 1.0f; // Poza okręgiem

		// Nałożenie Bounding Boxa wycinka
		int gridMinX = std::floor(std::max(rowMinX, bboxMinX));
		int gridMaxX = std::ceil(std::min(rowMaxX, bboxMaxX));

		// 3. Właściwa pętla po X (bardzo ograniczona)
		for (int x = gridMinX; x <= gridMaxX; ++x) {
			EMapCellState state = IsCellInSector(x + 0.5f, py, x + centerPosOffset.x + (y + centerPosOffset.y) * mapWidthInCells);
			if (state != EMapCellState::Invalid) {
				UpdateCell(position.x + x, position.y + y, state);
			}
		}
	}
}

