#pragma once

#include "simpleimage.h"

#include <vector>

class CSiftDescriptor
{ 
public:
	CSiftDescriptor(void);
	~CSiftDescriptor(void);

	/// Generate a descriptor based on the input data.
	/// param fx [in] 
	///
	/// return the descriptors.
	static std::vector<float> Generate(float fx, float fy, float angle, float scale, 
		CSimpleImagef &direction, CSimpleImagef &magnitude,
		int span = 4, int bincount = 4, float hicap = 0.2);
};
