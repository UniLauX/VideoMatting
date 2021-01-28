#pragma once

#include "KeyGen.h"
#include "../SimpleSIFT.h"

namespace GPUSIFT
{
	class CSiftGPUKeypoint;

	void setSensitivity_gpu(int iFeatureSens);

	extern float dog_threshold;
}

namespace SIMPLESIFT
{
	extern bool usegpu;
}


namespace SIFT
{

class SIFTKeyGen: public KeyGen
{
public:


public:

	SIFTKeyGen();
	~SIFTKeyGen();

	virtual bool Generate(ImageArrayf* img, KeyPointVector* kps);

	GPUSIFT::CSiftGPUKeypoint *m_siftgpu;

};

} // namespace SIFT