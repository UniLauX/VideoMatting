#pragma once

#include "simpleimage.h"
#include <vector>
#include "SimpleKeypoint.h"
#include "SiftFeaturePoint.h"

namespace SIMPLESIFT
{
	extern bool verbose;

	extern int threading;

	extern float presigma;
}

class CSimpleSIFT
{
public:
	CSimpleSIFT(void);
	~CSimpleSIFT(void);
	static std::vector<CSimpleKeypoint> Generate(CSimpleImagef simage, int method = OPENCV);
};
