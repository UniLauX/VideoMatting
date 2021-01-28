#pragma once

#include <vector>
#include "simpleimage.h"
#include "SimpleKeypoint.h"
#include "HarrisFeaturePoint.h"


class CHarrisSIFT
{
public:
	CHarrisSIFT(void);
	~CHarrisSIFT(void);

	static std::vector<CSimpleKeypoint> Generate(CSimpleImagef image, int threading);
};
