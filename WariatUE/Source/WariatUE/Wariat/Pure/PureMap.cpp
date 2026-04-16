#include "Wariat/Pure/PureMap.h"

namespace
{
void UeZero(uint8_t* ptr, int32 size)
{
	FMemory::Memzero(ptr, size);
}

uint8* UeRealloc(uint8_t* ptr, int32 size)
{
	return static_cast<uint8*>(FMemory::Realloc(ptr, size));
}

uint8* UeMalloc(int32 size)
{
	return static_cast<uint8*>(FMemory::Malloc(size));
}

void UeFree(uint8_t* ptr)
{
	FMemory::Free(ptr);
}
}

PureMap::PureMap()
	: ComMap({ &UeZero, &UeRealloc, &UeMalloc, &UeFree })
{
}