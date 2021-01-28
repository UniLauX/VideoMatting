#pragma once

#include <vector>
#include "simpleimage.h"

class CSiftRotation
{
public:
	CSiftRotation(void);
	~CSiftRotation(void);

	/// Generate a descriptor based on the input data.
	/// param fx [in] 
	///
	/// return the rotations.
	static std::vector<float> Generate(int centerx, int centery, float scale,
		CSimpleImagef &magnitude, CSimpleImagef &direction,
		int bincount = 36, float peakRelThresh = 0.8f);
};
