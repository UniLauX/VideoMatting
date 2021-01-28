#pragma once

#include <vector>
#include "SiftDescriptor.h"
#include "SiftRotation.h"
#include "SimpleKeypoint.h"
#include "SiftFeaturePoint.h"

namespace SIMPLESIFT
{class CMultiGenerator
{
public:
	CMultiGenerator(std::vector<FeaturePoint>* pfeatures, 
		std::vector<CSimpleImagef> *mags,
		std::vector<CSimpleImagef> *dirs,
		std::vector<CSimpleKeypoint> *keypoints,
		int start,
		int end);
	CMultiGenerator(const CMultiGenerator& object);

	~CMultiGenerator(void);

	void operator()();

	//////////////////////////////////////////////////////////////////////////
	std::vector<FeaturePoint> *_pfeatures;
	std::vector<CSimpleImagef> *_mags;
	std::vector<CSimpleImagef> *_dirs;

	//////////////////////////////////////////////////////////////////////////
	std::vector<CSimpleKeypoint> *_keypoints;

	//////////////////////////////////////////////////////////////////////////
	int _start;
	int _end;
};
}
