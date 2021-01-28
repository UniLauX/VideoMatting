#pragma once

#include <vector>
#include "simpleimage.h"

namespace HARRISFEATURE
{

extern int mindistance;
extern int maxcount;
extern float threshold;

class FeaturePoint
{
public:
	float fx;
	float fy;
	float scale;
};

class CHarrisFeaturePoint
{
public:
	CHarrisFeaturePoint(void);
	~CHarrisFeaturePoint(void);

	static std::vector<FeaturePoint> Generate(CSimpleImagef &image);
};
}
