#pragma once

#include "CoreMinimal.h"
#include "../../WariatCommon/Wariat.hpp"
#include "PureMap.h"
#include "PureInterface.hpp"
#include "../WariatUI.h"

class AUEWariat;

class WARIATUE_API PureWariat : public WariatCommon::Wariat<PureMap, UEMapRenderer, PureInterface>
{
	

public:
	PureWariat() {}
	~PureWariat();

};
