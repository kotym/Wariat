#pragma once

#include "CoreMinimal.h"

class AWariat;

class WARIATUE_API PureWariat
{
	AWariat* wariat = nullptr;
	

public:
	PureWariat() {}
	PureWariat(AWariat* wariatPtr);
	~PureWariat();

	void Update();
};
