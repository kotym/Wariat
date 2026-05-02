#pragma once
#include "ComMath.hpp"

class ComMap;

class ComNavi
{
public:
    ComNavi(const ComMap& comMap);
    ~ComNavi();

public:
    void Update(Vector2<float> pos, float rotation);

protected:
    const ComMap& map;   
};