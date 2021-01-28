#pragma once

#include "Others/annmt/include/ann/ANN.h"
#include "Others/annmt/lib/annlib.h"
#include <vector>
#include "SimpleKeypoint.h"

namespace SIMPLESIFT
{
class ANNMatcher
{
public:
	ANNMatcher(void);
	~ANNMatcher(void);

	void init(std::vector<CSimpleKeypoint> &kpv);
	bool find_match(const CSimpleKeypoint &kp, int nbCount, int* nbIndices, float* nbDists, float eps = 0.0f);


	void init(std::vector<std::vector<float> > &kpv);
	bool find_match(const std::vector<float> &kp, int nbCount, int* nbIndices, float* nbDists, float eps = 0.0f);

protected:
	std::vector<std::vector<float> > _keypoint;

	ANNkd_tree* mpTree;
	std::vector<ANNpoint> mvPts;
};
}
