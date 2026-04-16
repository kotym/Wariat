#include "Map.hpp"
#include <cstdlib>
#include <cstring>

namespace
{
void StdZero(uint8_t* ptr, int32_t size)
{
	std::memset(ptr, 0, static_cast<size_t>(size));
}

uint8_t* StdRealloc(uint8_t* ptr, int32_t size)
{
	return static_cast<uint8_t*>(std::realloc(ptr, static_cast<size_t>(size)));
}

uint8_t* StdMalloc(int32_t size)
{
	return static_cast<uint8_t*>(std::malloc(static_cast<size_t>(size)));
}

void StdFree(uint8_t* ptr)
{
	std::free(ptr);
}
}

Map::Map()
	: ComMap({ &StdZero, &StdRealloc, &StdMalloc, &StdFree })
{
}
