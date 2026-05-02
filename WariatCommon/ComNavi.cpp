#include "ComNavi.hpp"
#include "ComMap.hpp"
#include "ComMath.hpp"


ComNavi::ComNavi(const ComMap& comMap)
: map(comMap)
{
}

ComNavi::~ComNavi()
{
}

void ComNavi::Update(Vector2<float> pos, float rotation)
{
    EMapCellState cell = map.GetCellState(pos);

    // AABB

    // Detection forward
    const float sinA = sin(rotation);
    const float cosA = cos(rotation);

    const float wariatWidth = 1.5; //* 5 cm
    const float wDivCosA = wariatWidth / cosA;
    
    for (int32_t y = 0; y < 10; ++y)
    {
        for (int32_t x = 0; x < 10; ++x)
        {
            // is ahead of wariat
            if (y > -x * cosA / sinA)
                continue;
            // is 
        }
    }

    // Detection right
    const float wDivSinA = wariatWidth / sinA;
}