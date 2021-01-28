#pragma once

#include <vector>
#include "SimpleKeypoint.h"
#include "simpleimage.h"
#include "Vision\\Tracking\\FeatureTracking\\SiftGPU\\include\\SiftGPU.h"
//#include "Vision\\Tracking\\FeatureTracking\\SiftGPU\\lib\\siftgpulink.h"
namespace GPUSIFT
{
	void setSensitivity_gpu(int iFeatureSens);
	extern float dog_threshold;

class CSiftGPUKeypoint
{
public:
	CSiftGPUKeypoint(bool initglut);
	~CSiftGPUKeypoint(void);

	std::vector<CSimpleKeypoint> Generate(CSimpleImageb & image);

	void ClearGLContext();

	SiftGPU  *m_sift;

	bool m_initglut;
};
}

